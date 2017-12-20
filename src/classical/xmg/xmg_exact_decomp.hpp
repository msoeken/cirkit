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

/**
 * @file xmg_exact_decomp.hpp
 *
 * @brief Exact MAJ decomposition
 *
 * @author Mathias Soeken
 * @author Eleonora Testa
 * @since  2.4
 */

#ifndef XMG_EXACT_DECOMP_HPP
#define XMG_EXACT_DECOMP_HPP

#include <array>
#include <cstdint>
#include <iostream>
#include <vector>

#include <boost/optional.hpp>

#include <classical/abc/abc_api.hpp>
#include <classical/xmg/xmg.hpp>
#include <fmt/format.h>
#include <sat/glucose/AbcGlucose.h>

namespace cirkit
{

namespace detail
{

template<uint32_t NumVars, uint32_t NumGates>
struct xmg_exact_decomposition_impl
{
public:
  xmg_exact_decomposition_impl()
  {
    for ( auto& a : select )
    {
      for ( auto& a2 : a )
      {
        a2.fill( 0 );
      }
    }

    add_structure_variables();

    solver = abc::bmcg_sat_solver_start();
    abc::bmcg_sat_solver_set_nvars( solver, var_index );
  }

  ~xmg_exact_decomposition_impl()
  {
    abc::bmcg_sat_solver_stop( solver );
  }

  bool run()
  {
    if ( !add_main_cnf() )
    {
      return false;
    }

    // add value CNF for each interesting minterm
    for ( uint32_t i = 0; i < ( uint32_t( 1 ) << NumVars ); ++i )
    {
      auto cnt = __builtin_popcount( i );

      switch ( cnt )
      {
      default:
        break;
      case NumVars / 2 - 1:
        if ( !add_minterm( i, 0 ) )
        {
          return false;
        }
        break;
      case NumVars / 2 + 1:
        if ( !add_minterm( i, 1 ) )
        {
          return false;
        }
        break;
      case NumVars / 2:
        if ( !add_minterm( i, 2 ) )
        {
          return false;
        }
        break;
      }
    }

    if ( abc::bmcg_sat_solver_solve( solver, nullptr, 0 ) == GLUCOSE_SAT )
    {
      return true;
    }
    return false;
  }

  void print_statistics( std::ostream& os = std::cout )
  {
    os << fmt::format( "[i] vars = {}   cls = {}   learnt = {}   conf = {}",
                       abc::bmcg_sat_solver_varnum( solver ),
                       abc::bmcg_sat_solver_clausenum( solver ),
                       abc::bmcg_sat_solver_learntnum( solver ),
                       abc::bmcg_sat_solver_conflictnum( solver ) )
       << std::endl;
  }

  void print_solution( std::ostream& os = std::cout )
  {
    for ( auto i = 0; i < NumGates; ++i )
    {
      os << fmt::format( "n{} = <", i + NumVars + 1 );

      for ( auto k = 0; k < 3; ++k )
      {
        for ( auto j = 0; j < NumVars + i; ++j )
        {
          if ( select[i][k][j] && abc::bmcg_sat_solver_read_cex_varvalue( solver, select[i][k][j] ) )
          {
            os << fmt::format( "{}{}", j < NumVars ? 'x' : 'n', j + 1 );
          }
        }
      }

      os << ">" << std::endl;
    }
  }

  xmg_graph extract_circuit()
  {
    xmg_graph g;

    std::vector<xmg_function> functions;

    for ( auto i = 0; i < NumVars; ++i )
    {
      functions.push_back( g.create_pi( fmt::format( "x{}", i + 1 ) ) );
    }

    for ( auto i = 0; i < NumGates; ++i )
    {
      std::vector<xmg_function> children;

      for ( auto k = 0; k < 3; ++k )
      {
        for ( auto j = 0; j < NumVars + i; ++j )
        {
          if ( select[i][k][j] && abc::bmcg_sat_solver_read_cex_varvalue( solver, select[i][k][j] ) )
          {
            children.push_back( functions[j] );
          }
        }
      }

      assert( children.size() == 3u );

      functions.push_back( g.create_maj( children[0], children[1], children[2] ) );
    }

    const auto out = g.create_maj( g.create_pi( fmt::format( "x{}", NumVars ) ), functions[functions.size() - 2], functions[functions.size() - 1] );
    g.create_po( out, "f" );

    return g;
  }

private:
  void add_structure_variables()
  {
    /* first gate is <x1x2x3> */
    for ( auto k = 0; k < 3; ++k )
    {
      select[0][k][k] = var_index++;
    }

    for ( auto i = 1; i < NumGates; ++i )
    {
      for ( auto k = 0; k < 3; ++k )
      {
        const auto max = ( k == 0 ) ? NumVars : ( ( i == NumGates - 1 ) ? NumVars + i - 1 : NumVars + i );
        for ( auto j = 0; j < max; ++j )
        {
          select[i][k][j] = var_index;
          if ( j >= NumVars && ( j - NumVars ) < NumGates - 2 )
          {
            output_lits[j - NumVars].push_back( abc::Abc_Var2Lit( var_index, 0 ) );
          }
          var_index++;
        }
      }
    }
  }

  bool add_main_cnf()
  {
    /* every node has three children */
    for ( auto i = 0; i < NumGates; ++i )
    {
      for ( auto k = 0; k < 3; ++k )
      {
        std::vector<int> lits;

        for ( auto j = 0; j < NumVars + i; ++j )
        {
          if ( !select[i][k][j] )
            continue;

          lits.push_back( abc::Abc_Var2Lit( select[i][k][j], 0 ) );
        }

        if ( !abc::bmcg_sat_solver_addclause( solver, &lits[0], lits.size() ) )
        {
          return false;
        }

        for ( auto l = 1; l < lits.size(); ++l )
        {
          for ( auto m = 0; m < l; ++m )
          {
            int plits[] = {abc::Abc_LitNot( lits[m] ), abc::Abc_LitNot( lits[l] )};
            if ( !abc::bmcg_sat_solver_addclause( solver, plits, 2 ) )
            {
              return false;
            }
          }
        }

        /* TODO symmetry breaking */
        if ( k == 2 )
          break;

        for ( auto l = 1; l < NumVars + i; ++l )
        {
          for ( auto m = 0; m < l; ++m )
          {
            // select[i][k][l] -> !select[i][k + 1][m]
            if ( !select[i][k][l] || !select[i][k + 1][m] )
              continue;

            int plits[] = {abc::Abc_Var2Lit( select[i][k][l], 1 ), abc::Abc_Var2Lit( select[i][k + 1][m], 1 )};
            if ( !abc::bmcg_sat_solver_addclause( solver, plits, 2 ) )
            {
              return false;
            }
          }
        }
      }
    }

    /* every gate needs to be used */
    for ( auto& lits : output_lits )
    {
      if ( !abc::bmcg_sat_solver_addclause( solver, &lits[0], lits.size() ) )
      {
        return false;
      }
    }

    return true;
  }

  bool add_minterm( uint32_t minterm, int expected_value )
  {
    std::array<bool, NumVars> assignment;

    for ( auto i = 0; i < NumVars; ++i )
    {
      assignment[i] = ( minterm >> i ) & 1;
    }

    abc::bmcg_sat_solver_set_nvars( solver, var_index + 4 * NumGates );

    /* structure propagation */
    for ( auto i = 0; i < NumGates; ++i )
    {
      const auto base_var_i = var_index + 4 * i;

      for ( auto k = 0; k < 3; ++k )
      {

        for ( auto j = 0; j < NumVars + i; ++j )
        {
          if ( !select[i][k][j] )
            continue;

          std::vector<int> lits;
          lits.push_back( abc::Abc_Var2Lit( select[i][k][j], 1 ) );

          if ( j < NumVars ) /* node i'th k'th input should have value of PI j */
          {
            lits.push_back( abc::Abc_Var2Lit( base_var_i + k, assignment[j] ? 0 : 1 ) );
            if ( !abc::bmcg_sat_solver_addclause( solver, &lits[0], lits.size() ) )
            {
              return false;
            }
          }
          else /* node i'th k'th input should have value of node j */
          {
            const auto base_var_j = var_index + 4 * ( j - NumVars );
            lits.push_back( abc::Abc_Var2Lit( base_var_j + 3, 0 ) );
            lits.push_back( abc::Abc_Var2Lit( base_var_i + k, 1 ) );
            if ( !abc::bmcg_sat_solver_addclause( solver, &lits[0], lits.size() ) )
            {
              return false;
            }
            lits[1] = abc::Abc_LitNot( lits[1] );
            lits[2] = abc::Abc_LitNot( lits[2] );
            if ( !abc::bmcg_sat_solver_addclause( solver, &lits[0], lits.size() ) )
            {
              return false;
            }
          }
        }
      }

      for ( auto n = 0; n < 2; ++n )
      {
        if ( i >= NumGates - 2 && expected_value < 2 && n == expected_value )
        {
          continue;
        }

        for ( auto k = 0; k < 3; k++ )
        {
          int pLits[3], nLits = 0;
          if ( k != 0 )
          {
            pLits[nLits++] = abc::Abc_Var2Lit( base_var_i + 0, n );
          }
          if ( k != 1 )
          {
            pLits[nLits++] = abc::Abc_Var2Lit( base_var_i + 1, n );
          }
          if ( k != 2 )
          {
            pLits[nLits++] = abc::Abc_Var2Lit( base_var_i + 2, n );
          }
          if ( i < NumGates - 2 || expected_value == 2 )
          {
            pLits[nLits++] = abc::Abc_Var2Lit( base_var_i + 3, !n );
          }
          assert( nLits <= 3 );
          if ( !abc::bmcg_sat_solver_addclause( solver, pLits, nLits ) )
          {
            return false;
          }
        }
      }
    }

    /* special case for expected_value == 2 */
    if ( expected_value == 2 )
    {
      int pLits[2];
      pLits[0] = abc::Abc_Var2Lit( var_index + 4 * ( NumGates - 2 ) + 3, 0 );
      pLits[1] = abc::Abc_Var2Lit( var_index + 4 * ( NumGates - 1 ) + 3, 0 );
      if ( !abc::bmcg_sat_solver_addclause( solver, pLits, 2 ) )
      {
        return false;
      }
      pLits[0] = abc::Abc_LitNot( pLits[0] );
      pLits[1] = abc::Abc_LitNot( pLits[1] );
      if ( !abc::bmcg_sat_solver_addclause( solver, pLits, 2 ) )
      {
        return false;
      }
    }

    var_index += 4 * NumGates;

    return true;
  }

private:
  std::array<std::array<std::array<int, NumGates + NumVars - 1>, 3>, NumGates> select;
  std::array<std::vector<int>, NumGates - 2> output_lits;

  abc::bmcg_sat_solver* solver;

  int var_index = 1;
};
}

template<uint32_t NumVars, uint32_t NumGates>
boost::optional<xmg_graph> xmg_exact_decomposition()
{
  detail::xmg_exact_decomposition_impl<NumVars, NumGates> impl;
  if ( impl.run() )
  {
    impl.print_solution();
    impl.print_statistics();

    return impl.extract_circuit();
  }
  else
  {
    std::cout << "[i] no circuit found" << std::endl;
    return boost::none;
  }
}
}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
