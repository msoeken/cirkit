/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2017  EPFL
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "exact_toffoli_synthesis.hpp"

#include <array>
#include <iostream>

#include <boost/format.hpp>

#include <core/utils/timer.hpp>
#include <classical/abc/abc_api.hpp>
#include <reversible/functions/add_gates.hpp>
#include <reversible/utils/truth_table_helpers.hpp>

#include <sat/bsat/satSolver.h>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

class exact_toffoli_synthesis_manager
{
public:
  exact_toffoli_synthesis_manager( const binary_truth_table& spec, const properties::ptr& settings );

  void run( circuit& circ );

private:
  /* indexes:
   *   i    : target, line
   *   j, k : controls
   *   l    : gate
   *   t    : truth table row
   */
  int var_offset( int l ) const;
  int sim_var( int l, int i, int t ) const;
  int not_var( int l, int i ) const;
  int cnot_var( int l, int i, int j ) const;
  int tof_var( int l, int i, int j, int k ) const;

  int equal_other_clauses( int g, int l, int t, int i );
  int not_clauses( int l, int i, int t );
  int cnot_clauses( int l, int i, int j, int t );
  int tof_clauses( int l, int i, int j, int k, int t );
  int gate_clauses( int l );
  std::vector<int> spec_assumptions() const;
  void create_circuit( circuit& circ ) const;

  int symmetry_breaking_ordering( int l );
  int symmetry_breaking_not_same( int l );

  template<typename ...Lits>
  int add_clause( Lits... literals )
  {
    std::array<int, sizeof...( Lits )> cls = {{ literals... }};
    int * p = &cls.front();
    return abc::sat_solver_addclause( solver.get(), p, p + sizeof...( Lits ) );
  }

  int make_lit( int var, int c ) const;

  /* range helpers */
  template<typename Fn>
  void for_each_not( Fn&& f )
  {
    for ( auto i = 0u; i < n; ++i )
    {
      f( i );
    }
  }

  template<typename Fn>
  void for_each_cnot( Fn&& f )
  {
    for ( auto i = 0u; i < n; ++i )
    {
      for ( auto j = 1u; j < n; ++j )
      {
        f( i, j );
      }
    }
  }

  template<typename Fn>
  void for_each_tof( Fn&& f )
  {
    for ( auto i = 0u; i < n; ++i )
    {
      for ( auto j = 2u; j < n; ++j )
      {
        for ( auto k = 1u; k < j; ++k )
        {
          f( i, j, k );
        }
      }
    }
  }

  void debug_vars();

private:
  const binary_truth_table& spec;
  unsigned                  n, r;
  int                       num_vars_per_gate;
  int                       sim_offset;

  std::vector<boost::dynamic_bitset<>> spec_vec;

  /* solver */
  std::unique_ptr<abc::sat_solver, void(*)(abc::sat_solver*)> solver;

  /* settings */
  bool verbose = false;
};

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

exact_toffoli_synthesis_manager::exact_toffoli_synthesis_manager( const binary_truth_table& spec, const properties::ptr& settings )
  : spec( spec ),
    n( spec.num_inputs() ),
    solver( abc::sat_solver_new(), abc::sat_solver_delete ),
    verbose( get( settings, "verbose", verbose ) )
{
  /* we have:
     - 2^n             sim vars
     - n               NOT vars
     - n(n-1)          CNOT vars
     - n(n-1)(n-2) / 2 TOF vars
  */
  num_vars_per_gate = n * ( 1 << n ) + n + n * ( n - 1 ) + n * ( ( n - 1 ) * ( n - 2 ) ) / 2;
  sim_offset = n * ( 1 << n );

  spec_vec = truth_table_to_bitset_vector( spec );
}

void exact_toffoli_synthesis_manager::run( circuit& circ )
{
  r = 0;
  auto nvars = 0;
  abc::sat_solver_restart( solver.get() );

  while ( true )
  {
    ++r;
    nvars += num_vars_per_gate;

    std::cout << "[i] try to find optimum circuit with " << r << " gates" << std::endl;

    abc::sat_solver_setnvars( solver.get(), nvars );

    gate_clauses( r - 1 );

    if ( r > 1 )
    {
      symmetry_breaking_ordering( r - 1 );
      symmetry_breaking_not_same( r - 1 );
    }
    auto assump = spec_assumptions();
    int * p = &assump.front();

    const auto result = abc::sat_solver_solve( solver.get(), p, p + assump.size(), 0, 0, 0, 0 );

    if ( result == 1 ) /* SAT */
    {
      std::cout << "[i] found circuit with " << r << " gates" << std::endl;
      create_circuit( circ );
      return;
    }
  }

  assert( false );
}

inline int exact_toffoli_synthesis_manager::var_offset( int l ) const
{
  assert( l < static_cast<int>( r ) );

  return l * num_vars_per_gate;
}

inline int exact_toffoli_synthesis_manager::sim_var( int l, int i, int t ) const
{
  assert( l < static_cast<int>( r ) );
  assert( i < static_cast<int>( n ) );
  assert( t < static_cast<int>( 1u << n ) );

  return var_offset( l ) + i * ( 1 << n ) + t;
}

inline int exact_toffoli_synthesis_manager::not_var( int l, int i ) const
{
  assert( l < static_cast<int>( r ) );
  assert( i < static_cast<int>( n ) );

  return var_offset( l ) + sim_offset + i;
}

inline int exact_toffoli_synthesis_manager::cnot_var( int l, int i, int j ) const
{
  assert( l < static_cast<int>( r ) );
  assert( i < static_cast<int>( n ) );
  assert( j > 0 && j < static_cast<int>( n ) );

  return var_offset( l ) + sim_offset + n + i * ( n - 1 ) + j - 1;
}

inline int exact_toffoli_synthesis_manager::tof_var( int l, int i, int j, int k ) const
{
  assert( l < static_cast<int>( r ) );
  assert( i < static_cast<int>( n ) );
  assert( j > 1 && j < static_cast<int>( n ) );
  assert( k > 0 && k < static_cast<int>( j ) );

  auto offset = var_offset( l ) + sim_offset + n + n * ( n - 1 );
  offset += i * ( ( n - 2 ) * ( n - 1 ) ) / 2; /* offset based on target */
  offset += ( ( j - 2 ) * ( j - 1 ) ) / 2; /* right j entry */

  return offset + k - 1;
}

inline int exact_toffoli_synthesis_manager::equal_other_clauses( int g, int l, int t, int i )
{
  for ( auto ii = 0u; ii < n; ++ii )
  {
    if ( static_cast<int>( ii ) == i ) continue;

    if ( l == 0 )
    {
      const auto bit = ( t >> ii ) & 1;
      add_clause( make_lit( g, 1 ), make_lit( sim_var( l, ii, t ), 1 - bit ) );
    }
    else
    {
      add_clause( make_lit( g, 1 ), make_lit( sim_var( l - 1, ii, t ), 0 ), make_lit( sim_var( l, ii, t ), 1 ) );
      add_clause( make_lit( g, 1 ), make_lit( sim_var( l - 1, ii, t ), 1 ), make_lit( sim_var( l, ii, t ), 0 ) );
    }
  }

  return 1;
}

int exact_toffoli_synthesis_manager::not_clauses( int l, int i, int t )
{
  const auto g = not_var( l, i );
  equal_other_clauses( g, l, t, i );

  if ( l == 0 )
  {
    const auto bit = ( t >> i ) & 1;
    add_clause( make_lit( g, 1 ), make_lit( sim_var( l, i, t ), bit ) );
  }
  else
  {
    add_clause( make_lit( g, 1 ), make_lit( sim_var( l - 1, i, t ), 0 ), make_lit( sim_var( l, i, t ), 0 ) );
    add_clause( make_lit( g, 1 ), make_lit( sim_var( l - 1, i, t ), 1 ), make_lit( sim_var( l, i, t ), 1 ) );
  }

  return 1;
}

int exact_toffoli_synthesis_manager::cnot_clauses( int l, int i, int j, int t )
{
  const auto g = cnot_var( l, i, j );
  equal_other_clauses( g, l, t, i );

  const auto c1 = ( i + j ) % n;

  if ( l == 0 )
  {
    const auto tbit = ( t >> i ) & 1;
    const auto cbit = ( t >> c1 ) & 1;
    add_clause( make_lit( g, 1 ), make_lit( sim_var( l, i, t ), tbit == cbit ) );
  }
  else
  {
    add_clause( make_lit( g, 1 ), make_lit( sim_var( l - 1, i, t ), 0 ), make_lit( sim_var( l - 1, c1, t ), 0 ), make_lit( sim_var( l, i, t ), 1 ) );
    add_clause( make_lit( g, 1 ), make_lit( sim_var( l - 1, i, t ), 0 ), make_lit( sim_var( l - 1, c1, t ), 1 ), make_lit( sim_var( l, i, t ), 0 ) );
    add_clause( make_lit( g, 1 ), make_lit( sim_var( l - 1, i, t ), 1 ), make_lit( sim_var( l - 1, c1, t ), 0 ), make_lit( sim_var( l, i, t ), 0 ) );
    add_clause( make_lit( g, 1 ), make_lit( sim_var( l - 1, i, t ), 1 ), make_lit( sim_var( l - 1, c1, t ), 1 ), make_lit( sim_var( l, i, t ), 1 ) );
  }

  return 1;
}

int exact_toffoli_synthesis_manager::tof_clauses( int l, int i, int j, int k, int t )
{
  const auto g = tof_var( l, i, j, k );
  equal_other_clauses( g, l, t, i );

  const auto c1 = ( i + j ) % n;
  const auto c2 = ( i + k ) % n;

  if ( l == 0 )
  {
    const auto tbit = ( t >> i ) & 1;
    const auto cbit1 = ( t >> c1 ) & 1;
    const auto cbit2 = ( t >> c2 ) & 1;
    add_clause( make_lit( g, 1 ), make_lit( sim_var( l, i, t ), tbit == ( cbit1 && cbit2 ) ) );
  }
  else
  {
    add_clause( make_lit( g, 1 ), make_lit( sim_var( l - 1, i, t ), 1 ), make_lit( sim_var( l - 1, c1, t ), 0 ), make_lit( sim_var( l, i, t ), 0 ) );
    add_clause( make_lit( g, 1 ), make_lit( sim_var( l - 1, i, t ), 0 ), make_lit( sim_var( l - 1, c1, t ), 0 ), make_lit( sim_var( l, i, t ), 1 ) );
    add_clause( make_lit( g, 1 ), make_lit( sim_var( l - 1, i, t ), 1 ), make_lit( sim_var( l - 1, c2, t ), 0 ), make_lit( sim_var( l, i, t ), 0 ) );
    add_clause( make_lit( g, 1 ), make_lit( sim_var( l - 1, i, t ), 0 ), make_lit( sim_var( l - 1, c2, t ), 0 ), make_lit( sim_var( l, i, t ), 1 ) );
    add_clause( make_lit( g, 1 ), make_lit( sim_var( l - 1, i, t ), 1 ), make_lit( sim_var( l - 1, c1, t ), 1 ), make_lit( sim_var( l - 1, c2, t ), 1 ), make_lit( sim_var( l, i, t ), 1 ) );
    add_clause( make_lit( g, 1 ), make_lit( sim_var( l - 1, i, t ), 0 ), make_lit( sim_var( l - 1, c1, t ), 1 ), make_lit( sim_var( l - 1, c2, t ), 1 ), make_lit( sim_var( l, i, t ), 0 ) );
  }

  return 1;
}

int exact_toffoli_synthesis_manager::symmetry_breaking_ordering( int l )
{
  /* no CNOT( i, j ) before NOT( i ) */
  for_each_cnot( [this, l]( int i, int j ) {
      add_clause( make_lit( not_var( l, i ), 1 ), make_lit( cnot_var( l - 1, i, j ), 1 ) );
    } );
  /* no TOF( i, j, k ) before NOT( i ) */
  for_each_tof( [this, l]( int i, int j, int k ) {
      add_clause( make_lit( not_var( l, i ), 1 ), make_lit( tof_var( l - 1, i, j, k ), 1 ) );
    } );
  /* order CNOTs with same target */
  for_each_cnot( [this, l]( int i, int j ) {
      for ( auto jj = 1; jj < j; ++jj )
      {
        add_clause( make_lit( cnot_var( l, i, j ), 1 ), make_lit( cnot_var( l - 1, i, jj ), 1 ) );
      }
    } );
  /* order of TOFs with same target */
  for_each_tof( [this, l]( int i, int j, int k ) {
      for ( auto jj = 2; jj < j; ++jj )
      {
        for ( auto kk = 1; kk < jj; ++kk )
        {
          add_clause( make_lit( tof_var( l, i, j, k ), 1 ), make_lit( tof_var( l - 1, i, jj, kk ), 1 ) );
        }
      }
      for ( auto kk = 1; kk < j; ++kk )
      {
        add_clause( make_lit( tof_var( l, i, j, k ), 1 ), make_lit( tof_var( l - 1, i, j, kk ), 1 ) );
      }
    } );
  /* no CNOT( i, j ) before NOT( i' ) where i' not in {i, j} */
  for_each_cnot( [this, l]( int i, int j ) {
      for ( auto ii = 0u; ii < n; ++ii )
      {
        if ( static_cast<int>( ii ) == i || ii == ( ( i + j ) % n ) ) { continue; }
        add_clause( make_lit( not_var( l, ii ), 1 ), make_lit( cnot_var( l - 1, i, j ), 1 ) );
      }
    } );
  /* no TOF( i, j, k ) before NOT( i' ) where i' not in {i, j, k} */
  for_each_tof( [this, l]( int i, int j, int k ) {
      for ( auto ii = 0u; ii < n; ++ii )
      {
        if ( static_cast<int>( ii ) == i || ii == ( ( i + j ) % n ) || ii == ( ( i + k ) % n ) ) { continue; }
        add_clause( make_lit( not_var( l, ii ), 1 ), make_lit( tof_var( l - 1, i, j, k ), 1 ) );
      }
    } );

  return 1;
}

int exact_toffoli_synthesis_manager::symmetry_breaking_not_same( int l )
{
  for_each_not( [this, l]( int i ) {
      add_clause( make_lit( not_var( l, i ), 1 ), make_lit( not_var( l - 1, i ), 1 ) );
    } );
  for_each_cnot( [this, l]( int i, int j ) {
      add_clause( make_lit( cnot_var( l, i, j ), 1 ), make_lit( cnot_var( l - 1, i, j ), 1 ) );
    } );
  for_each_tof( [this, l]( int i, int j, int k ) {
      add_clause( make_lit( tof_var( l, i, j, k ), 1 ), make_lit( tof_var( l - 1, i, j, k ), 1 ) );
    } );

  return 1;
}

void exact_toffoli_synthesis_manager::debug_vars()
{
  for ( auto l = 0u; l < r; ++l )
  {
    for ( auto i = 0u; i < n; ++i )
    {
      for ( auto t = 0u; t < ( 1u << n ); ++t )
      {
        std::cout << boost::format( "%6d sim_var( %d, %d, %d )" ) % sim_var( l, i, t ) % l % i % t << std::endl;
      }
    }

    for ( auto i = 0u; i < n; ++i )
    {
      std::cout << boost::format( "%6d not_var( %d, %d )" ) % not_var( l, i ) % l % i << std::endl;
    }

    for ( auto i = 0u; i < n; ++i )
    {
      for ( auto j = 1u; j < n; ++j )
      {
        std::cout << boost::format( "%6d cnot_var( %d, %d, %d )" ) % cnot_var( l, i, j ) % l % i % j << std::endl;
      }
    }

    for ( auto i = 0u; i < n; ++i )
    {
      for ( auto j = 2u; j < n; ++j )
      {
        for ( auto k = 1u; k < j; ++k )
        {
          std::cout << boost::format( "%6d tof_var( %d, %d, %d, %d )" ) % tof_var( l, i, j, k ) % l % i % j % k << std::endl;
        }
      }
    }
  }
}

int exact_toffoli_synthesis_manager::gate_clauses( int l )
{
  for ( auto t = 0u; t < ( 1u << n ); ++t )
  {
    for_each_not( [this, l, t]( int i ) { not_clauses( l, i, t ); } );
    for_each_cnot( [this, l, t]( int i, int j ) { cnot_clauses( l, i, j, t ); } );
    for_each_tof( [this, l, t]( int i, int j, int k ) { tof_clauses( l, i, j, k, t ); } );
  }

  /* at least one gate */
  std::vector<int> gates;

  for ( auto i = 0u; i < n; ++i )
  {
    gates.push_back( make_lit( not_var( l, i ), 0 ) );

    for ( auto j = 1u; j < n; ++j )
    {
      gates.push_back( make_lit( cnot_var( l, i, j ), 0 ) );

      for ( auto k = 1u; k < j; ++k )
      {
        gates.push_back( make_lit( tof_var( l, i, j, k ), 0 ) );
      }
    }
  }

  int * p = &gates.front();
  abc::sat_solver_addclause( solver.get(), p, p + gates.size() );

  return 1;
}

std::vector<int> exact_toffoli_synthesis_manager::spec_assumptions() const
{
  std::vector<int> assumptions;
  assumptions.reserve(  sim_offset );

  for ( auto i = 0u; i < n; ++i )
  {
    for ( auto t = 0u; t < ( 1u << n ); ++t )
    {
      assumptions.push_back( make_lit( sim_var( r - 1, i, t ), spec_vec[t][i] ? 0 : 1 ) );
    }
  }

  return assumptions;
}

void exact_toffoli_synthesis_manager::create_circuit( circuit& circ ) const
{
  circ.set_lines( n );

  for ( auto l = 0u; l < r; ++l )
  {
    auto added = false;

    for ( auto i = 0u; i < n; ++i )
    {
      if ( abc::sat_solver_var_value( solver.get(), not_var( l, i ) ) )
      {
        if ( !added )
        {
          added = true;
          append_not( circ, n - i - 1 );
        }
        else { std::cout << "[w] alternative gate found" << std::endl; }
      }

      for ( auto j = 1u; j < n; ++j )
      {
        if ( abc::sat_solver_var_value( solver.get(), cnot_var( l, i, j ) ) )
        {
          if ( !added )
          {
            added = true;
            append_cnot( circ, n - ( ( i + j ) % n ) - 1, n - i - 1 );
          }
          else { std::cout << "[w] alternative gate found" << std::endl; }
        }

        for ( auto k = 1u; k < j; ++k )
        {
          if ( abc::sat_solver_var_value( solver.get(), tof_var( l, i, j, k ) ) )
          {
            if ( !added )
            {
              added = true;
              append_toffoli( circ )( n - ( ( i + k ) % n ) - 1, n - ( ( i + j ) % n ) - 1 )( n - i - 1 );
            }
            else { std::cout << "[w] alternative gate found" << std::endl; }
          }
        }
      }
    }
  }
}

inline int exact_toffoli_synthesis_manager::make_lit( int var, int c ) const
{
  return abc::Abc_Var2Lit( var, c );
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

bool exact_toffoli_synthesis( circuit& circ, const binary_truth_table& spec, const properties::ptr& settings, const properties::ptr& statistics )
{
  properties_timer t( statistics );

  exact_toffoli_synthesis_manager mgr( spec, settings );

  mgr.run( circ );

  return true;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
