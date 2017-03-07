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

#include "aig_npn_canonization.hpp"

#include <boost/format.hpp>
#include <boost/range/algorithm_ext/iota.hpp>
#include <boost/range/algorithm_ext/copy_n.hpp>

#include <core/utils/bitset_utils.hpp>
#include <core/utils/range_utils.hpp>
#include <core/utils/terminal.hpp>
#include <core/utils/timer.hpp>
#include <classical/functions/aig_cone.hpp>
#include <classical/functions/simulate_aig.hpp>
#include <classical/functions/strash.hpp>
#include <classical/io/write_aiger.hpp>
#include <classical/utils/aig_utils.hpp>
#include <classical/utils/truth_table_utils.hpp>
#include <classical/sat/sat_solver.hpp>
#include <classical/sat/minisat.hpp>
#include <classical/sat/utils/add_aig.hpp>
#include <classical/sat/utils/add_aig_with_gia.hpp>
#include <classical/sat/utils/lexicographic.hpp>
#include <classical/sat/utils/visit_solutions.hpp>

#define timer timer_class
#include <boost/progress.hpp>
#undef timer

#define L(x) if ( verbose ) { std::cout << x << std::endl; }

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

std::map<unsigned, unsigned> make_permutation( const std::vector<unsigned>& perm )
{
  std::map<unsigned, unsigned> perm_t;
  for ( auto i = 0u; i < perm.size(); ++i )
  {
    perm_t.insert( {perm[i], i} );
  }
  return perm_t;
}

std::map<unsigned, unsigned> translate_permutation( const std::vector<unsigned>& perm, const boost::dynamic_bitset<>& mapped_inputs )
{
  std::vector<unsigned> mapped_indexes;

  foreach_bit( mapped_inputs, [&]( unsigned pos ) {
      mapped_indexes.push_back( pos );
    } );

  std::map<unsigned, unsigned> perm_t;
  auto pos = 0u;
  for ( auto i = 0u; i < mapped_inputs.size(); ++i )
  {
    if ( mapped_inputs[i] )
    {
      //perm_t.insert( {i, mapped_indexes[perm[pos]]} );
      perm_t.insert( {mapped_indexes[perm[pos]], i} );
      pos++;
    }
    else
    {
      perm_t.insert( {i, i} );
    }
  }

  return perm_t;
}

boost::dynamic_bitset<> translate_phase( const boost::dynamic_bitset<>& phase, const boost::dynamic_bitset<>& mapped_inputs )
{
  boost::dynamic_bitset<> phase_t( mapped_inputs.size() );

  auto i = 0u;
  foreach_bit( mapped_inputs, [&]( unsigned pos ) {
      phase_t[pos] = phase[i++];
    } );
  return phase_t;
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

class aig_npn_canonization_manager
{
public:
  aig_npn_canonization_manager( const aig_graph& aig,
                                boost::dynamic_bitset<>& phase,
                                std::vector<unsigned>& perm,
                                const properties::ptr& settings )
      : aig( aig ), info( aig_info( aig ) ), phase( phase ), perm( perm )
  {
    /* initialize phase and perm */
    const auto n = info.inputs.size();
    phase.resize( n + 1u );
    phase.reset();
    perm.resize( n );
    boost::iota( perm, 0u );

    phase_next = phase;
    perm_next  = perm;

    encoding = get( settings, "encoding", 0u ); /* 0: Tseytin, 1: EMS */
    precheck = get( settings, "precheck", false );

    if ( precheck )
    {
      current_minterm = compute_minterm();
    }
  }

  void reset( bool output_phase )
  {
    phase.reset();
    phase.set( info.inputs.size(), output_phase );
    boost::iota( perm, 0u );

    phase_next = phase;
    perm_next  = perm;
    recompute  = true;

    if ( precheck )
    {
      current_minterm = compute_minterm();
    }
  }

  bool try_flip( unsigned i )
  {
    phase_next.flip( i );

    if ( lexicographically_larger() )
    {
      phase.flip( i );
      recompute = true;
      return true;
    }
    else
    {
      phase_next.flip( i );
      return false;
    }
  }

  bool try_swap( unsigned i, unsigned j )
  {
    std::swap( perm_next[i], perm_next[j] );

    if ( lexicographically_larger() )
    {
      std::swap( perm[i], perm[j] );
      recompute = true;
      return true;
    }
    else
    {
      std::swap( perm_next[i], perm_next[j] );
      return false;
    }
  }

  bool try_sift( unsigned i )
  {
    auto improvement = false;

    assert( perm == perm_next );
    assert( phase == phase_next );

    for ( auto k = 1u; k < 8u; ++k )
    {
      if ( k % 4u == 0u )
      {
        std::swap( perm_next[i], perm_next[i + 1] );
      }
      else if ( k % 2u == 0 )
      {
        phase_next.flip( i + 1 );
      }
      else
      {
        phase_next.flip( i );
      }

      if ( lexicographically_larger() )
      {
        recompute = true;
        perm = perm_next;
        phase = phase_next;
        improvement = true;
      }
    }

    if ( improvement )
    {
      perm_next = perm;
      phase_next = phase;
      recompute = true;
      return true;
    }
    else
    {
      /* last cycle returns to original */
      std::swap( perm_next[i], perm_next[i + 1] );
      assert( perm == perm_next );
      assert( phase == phase_next );
      return false;
    }
  }

  bool try_explicit( const std::vector<unsigned>& other_perm, const boost::dynamic_bitset<>& other_phase )
  {
    perm_next = other_perm;
    phase_next = other_phase;

    if ( lexicographically_larger() )
    {
      perm = perm_next;
      phase = phase_next;
      recompute = true;
      return true;
    }
    else
    {
      perm_next = perm;
      phase_next = phase;
      return false;
    }
  }

private:
  aig_graph build_lex_miter()
  {
    const auto n = phase.size() - 1u;
    auto settings = std::make_shared<properties>();

    if ( recompute )
    {
      settings->set( "reorder", make_permutation( perm ) );
      settings->set( "invert",  sub_bitset( phase, 0u, n ) );
      current = strash( aig, settings );
      recompute = false;
    }
    auto miter = current;

    settings->set( "reorder", make_permutation( perm_next ) );
    settings->set( "invert",  sub_bitset( phase_next, 0u, n ) );
    settings->set( "reuse_inputs", true );
    strash( aig, miter, settings );

    auto& miter_info = aig_info( miter );
    assert( miter_info.inputs.size() == n );
    assert( miter_info.outputs.size() == 2u );

    const auto f1 = miter_info.outputs[0u].first;
    const auto f2 = miter_info.outputs[1u].first;

    const auto f = aig_create_xor( miter, f1 ^ phase[n], f2 ^ phase_next[n] );
    miter_info.outputs.clear();
    aig_create_po( miter, f, "output" );

    return miter;
  }

  bool lexicographically_larger()
  {
    /* pre-check smallest minterm */
    if ( precheck )
    {
      auto minterm = compute_minterm();
      if ( minterm < current_minterm )
      {
        current_minterm = minterm;
        return true;
      }
      else if ( minterm > current_minterm )
      {
        return false;
      }
    }

    /* more expensive miter */
    aig_graph        miter;
    minisat_solver   solver;
    std::vector<int> piids, poids;

    {
      increment_timer t( &miter_runtime );
      miter = build_lex_miter();
    }

    {
      increment_timer t( &encoding_runtime );
      solver = make_solver<minisat_solver>();

      if ( encoding == 0 )
      {
        add_aig( solver, miter, 1, piids, poids );
      }
      else
      {
        add_aig_with_gia( solver, miter, 1, piids, poids );
      }
    }

    try
    {
      const auto minterm = lexicographic_largest_solution( solver, piids, poids, properties::ptr(), statistics );

      ++lexsat_calls;
      sat_calls += statistics->get<unsigned>( "sat_calls" );
      runtime   += statistics->get<double>( "runtime" );

      simple_node_assignment_simulator::aig_node_value_map map;
      for ( auto i = 0u; i < perm.size(); ++i )
      {
        //map[info.inputs[perm[i]]] = minterm[i] != phase[i]; /* != is XOR */
        map[info.inputs[perm[i]]] = minterm[i] != phase[perm[i]];
      }
      const auto simval = simulate_aig( aig, simple_node_assignment_simulator( map ) ).at( info.outputs[0u].first );
      if ( simval != phase[perm.size()] ) /* != is XOR */
      {
        current_minterm = minterm;
        return true;
      }
      else
      {
        return false;
      }
    }
    catch ( const unsat_exception& e )
    {
      return false;
    }
  }

  boost::dynamic_bitset<> compute_minterm()
  {
    minisat_solver solver;
    std::vector<int> piids, poids;

    auto settings = std::make_shared<properties>();
    settings->set( "reorder", make_permutation( perm_next ) );
    settings->set( "invert",  sub_bitset( phase_next, 0u, perm_next.size() ) );
    const auto reordered = strash( aig, settings );

    {
      increment_timer t( &encoding_runtime );
      solver = make_solver<minisat_solver>();

      if ( encoding == 0 )
      {
        add_aig( solver, reordered, 1, piids, poids );
      }
      else
      {
        add_aig_with_gia( solver, reordered, 1, piids, poids );
      }
    }

    if ( phase_next[perm_next.size()] )
    {
      poids[0] = -poids[0];
    }

    const auto minterm = lexicographic_largest_solution( solver, piids, poids, properties::ptr(), statistics );

    ++lexsat_calls;
    sat_calls += statistics->get<unsigned>( "sat_calls" );
    runtime   += statistics->get<double>( "runtime" );

    return minterm;
  }

private:
  const aig_graph&         aig;
  const aig_graph_info&    info;
  boost::dynamic_bitset<>& phase;
  std::vector<unsigned>&   perm;
  boost::dynamic_bitset<>  phase_next;
  std::vector<unsigned>    perm_next;
  aig_graph                current;
  bool                     recompute = true;
  unsigned                 encoding = 0u;
  bool                     precheck = false;

  properties::ptr          statistics = std::make_shared<properties>();

  boost::dynamic_bitset<>  current_minterm;

public:
  unsigned long            sat_calls        = 0ul;
  unsigned long            lexsat_calls     = 0ul;
  double                   runtime          = 0.0;
  double                   miter_runtime    = 0.0;
  double                   encoding_runtime = 0.0;
};

class aig_npn_canonization_miter_simulator : public aig_simulator<aig_function>
{
public:
  aig_npn_canonization_miter_simulator( aig_graph& miter, unsigned n, unsigned offset )
    : miter( miter ),
      miter_info( aig_info( miter ) ),
      num_inputs( n ),
      offset( offset ) {}

  aig_function get_input( const aig_node& node, const std::string& name, unsigned pos, const aig_graph& aig ) const
  {
    const auto i1 = pos + offset;
    const auto i2 = pos + ( num_inputs << 1u ) + offset;
    //std::cout << boost::format( "[i] xor for %d and %d" ) % i1 % i2 << std::endl;
    return aig_create_xor( miter, {miter_info.inputs[i1], false}, {miter_info.inputs[i2], false} );
  }

  aig_function get_constant() const
  {
    return aig_get_constant( miter, false );
  }

  aig_function invert( const aig_function& v ) const
  {
    return !v;
  }

  aig_function and_op( const aig_node& node, const aig_function& v1, const aig_function& v2 ) const
  {
    return aig_create_and( miter, v1, v2 );
  }

private:
  aig_graph& miter;
  aig_graph_info miter_info;
  unsigned num_inputs;
  unsigned offset;
};

class aig_npn_canonization_shared_miter_manager
{
public:
  aig_npn_canonization_shared_miter_manager( const aig_graph& aig,
                                             boost::dynamic_bitset<>& phase,
                                             std::vector<unsigned>& perm,
                                             const properties::ptr& settings )
    : aig( aig ), info( aig_info( aig ) ), phase( phase ), perm( perm ),
      vars( info.inputs.size() ),
      polarities( info.inputs.size() << 1u ),
      pairs( info.inputs.size() * info.inputs.size() )
  {
    encoding = get( settings, "encoding", 0u ); /* 0: Tseytin, 1: EMS */

    /* initialize phase and perm */
    const auto n = info.inputs.size();
    phase.resize( n + 1u );
    phase.reset();
    perm.resize( n );
    boost::iota( perm, 0u );

    phase_next = phase;
    perm_next  = perm;

    build_miter();
    build_solver();
  }

  void reset( bool output_phase )
  {
    phase.reset();
    phase.set( info.inputs.size(), output_phase );
    boost::iota( perm, 0u );

    phase_next = phase;
    perm_next  = perm;
  }

  bool try_flip( unsigned i )
  {
    phase_next.flip( i );

    if ( lexicographically_larger() )
    {
      phase.flip( i );
      return true;
    }
    else
    {
      phase_next.flip( i );
      return false;
    }
  }

  bool try_swap( unsigned i, unsigned j )
  {
    std::swap( perm_next[i], perm_next[j] );

    if ( lexicographically_larger() )
    {
      std::swap( perm[i], perm[j] );
      return true;
    }
    else
    {
      std::swap( perm_next[i], perm_next[j] );
      return false;
    }
  }

  bool try_sift( unsigned i )
  {
    auto improvement = false;

    assert( perm == perm_next );
    assert( phase == phase_next );

    for ( auto k = 1u; k < 8u; ++k )
    {
      if ( k % 4u == 0u )
      {
        std::swap( perm_next[i], perm_next[i + 1] );
      }
      else if ( k % 2u == 0 )
      {
        phase_next.flip( i + 1 );
      }
      else
      {
        phase_next.flip( i );
      }

      if ( lexicographically_larger() )
      {
        perm = perm_next;
        phase = phase_next;
        improvement = true;
      }
    }

    if ( improvement )
    {
      perm_next = perm;
      phase_next = phase;
      return true;
    }
    else
    {
      /* last cycle returns to original */
      std::swap( perm_next[i], perm_next[i + 1] );
      assert( perm == perm_next );
      assert( phase == phase_next );
      return false;
    }
  }

  bool try_explicit( const std::vector<unsigned>& other_perm, const boost::dynamic_bitset<>& other_phase )
  {
    perm_next = other_perm;
    phase_next = other_phase;

    if ( lexicographically_larger() )
    {
      perm = perm_next;
      phase = phase_next;
      return true;
    }
    else
    {
      perm_next = perm;
      phase_next = phase;
      return false;
    }
  }

private:
  void build_miter()
  {
    reference_timer t( &miter_runtime );

    const auto n = perm.size();

    aig_initialize( miter );
    auto& miter_info = aig_info( miter );

    for ( auto i = 0u; i < ( n << 2u ); ++i )
    {
      aig_create_pi( miter, boost::str( boost::format( "pi%d" ) % i ) );
    }

    const auto f1 = simulate_aig( aig, aig_npn_canonization_miter_simulator( miter, n, 0u ) ).at( info.outputs[0u].first );
    const auto f2 = simulate_aig( aig, aig_npn_canonization_miter_simulator( miter, n, n ) ).at( info.outputs[0u].first );

    const auto f = aig_create_xor( miter, f1, f2 );
    aig_create_po( miter, f, "output" );

    for ( auto i = 0u; i < n; ++i )
    {
      for ( auto j = 0u; j < n; ++j )
      {
        //std::cout << boost::format( "[i] xnor for %d and %d" ) % i % j << std::endl;
        aig_create_po( miter,
                       !aig_create_xor( miter, {miter_info.inputs[i], false},
                                        {miter_info.inputs[n + j], false} ),
                       boost::str( boost::format( "xnor-%d-%d" ) % i % j ) );
      }
    }
  }

  void build_solver()
  {
    reference_timer t( &encoding_runtime );

    const auto n = perm.size();

    solver = make_solver<minisat_solver>();

    std::vector<int> piids, poids;

    if ( encoding == 0 )
    {
      add_aig( solver, miter, 1, piids, poids );
    }
    else
    {
      add_aig_with_gia( solver, miter, 1, piids, poids );
    }

    /* variables */
    boost::copy_n( piids, n, vars.begin() );

    /* miter output */
    miter_output = poids.front();

    /* polarities */
    std::copy( piids.begin() + ( n << 1u ), piids.end(), polarities.begin() );

    /* input pairs */
    std::copy( poids.begin() + 1u, poids.end(), pairs.begin() );
  }

  bool lexicographically_larger()
  {
    const auto n = perm.size();

    try
    {
      std::vector<int> assumptions;

      std::vector<int> lvars( n );

      // std::vector<unsigned> inv( n );
      // for ( auto i = 0u; i < n; ++i )
      // {
      //   inv[perm_next[i]] = i;
      // }

      for ( auto i = 0u; i < n; ++i )
      {
        //lvars[perm_next[i]] = vars[i];
        lvars[i] = vars[perm[i]];
      }

      assumptions.push_back( miter_output * ( ( phase[n] != phase_next[n] ) ? -1 : 1 ) );

      /* perm */
      for ( auto i = 0u; i < n; ++i )
      {
        assumptions.push_back( pairs[perm[i] * n + perm_next[i]] );
        assumptions.push_back( polarities[i] * ( phase[i] ? 1 : -1 ) );
        assumptions.push_back( polarities[i + n] * ( phase_next[i] ? 1 : -1 ) );
      }

      const auto minterm = lexicographic_largest_solution( solver, lvars, assumptions, properties::ptr(), statistics );

      ++lexsat_calls;
      sat_calls += statistics->get<unsigned>( "sat_calls" );
      runtime   += statistics->get<double>( "runtime" );

      simple_node_assignment_simulator::aig_node_value_map map;
      for ( auto i = 0u; i < perm.size(); ++i )
      {
        //map[info.inputs[perm[i]]] = minterm[i] != phase[i]; /* != is XOR */
        map[info.inputs[perm[i]]] = minterm[i] != phase[perm[i]];
      }
      const auto simval = simulate_aig( aig, simple_node_assignment_simulator( map ) ).at( info.outputs[0u].first );
      return simval != phase[perm.size()];
    }
    catch ( const unsat_exception& e )
    {
      return false;
    }
  }

private:
  const aig_graph&         aig;
  const aig_graph_info&    info;
  unsigned                 encoding = 0u;
  boost::dynamic_bitset<>& phase;
  std::vector<unsigned>&   perm;
  boost::dynamic_bitset<>  phase_next;
  std::vector<unsigned>    perm_next;
  aig_graph                miter;
  minisat_solver           solver;
  std::vector<int>         vars;
  int                      miter_output;
  std::vector<int>         polarities;
  std::vector<int>         pairs;

  properties::ptr          statistics = std::make_shared<properties>();

public:
  unsigned long            sat_calls        = 0ul;
  unsigned long            lexsat_calls     = 0ul;
  double                   runtime          = 0.0;
  double                   miter_runtime    = 0.0;
  double                   encoding_runtime = 0.0;
};

template<typename Manager>
void fill_statistics( const Manager& mgr, const properties::ptr& statistics )
{
  set( statistics, "lexsat_calls", mgr.lexsat_calls );
  set( statistics, "sat_calls", mgr.sat_calls );
  set( statistics, "sat_runtime", mgr.runtime );
  set( statistics, "miter_runtime", mgr.miter_runtime );
  set( statistics, "encoding_runtime", mgr.encoding_runtime );
}

template<typename Manager>
void aig_npn_canonization_flip_swap_generic( const aig_graph& aig, boost::dynamic_bitset<>& phase, std::vector<unsigned>& perm,
                                             const properties::ptr& settings,
                                             const properties::ptr& statistics )
{
  /* settings */
  const auto verbose = get( settings, "verbose", false );

  /* timing */
  properties_timer t( statistics );

  const auto& info = aig_info( aig );

  assert( info.outputs.size() == 1u ); /* assume single output AIG */

  Manager mgr( aig, phase, perm, settings );

  auto improvement = true;
  auto round = 0u;
  const auto n = info.inputs.size();
  while ( improvement )
  {
    L( "[i] round " << ++round );

    improvement = false;

    /* input/output negation */
    for ( auto i = 0u; i <= n; ++i )
    {
      if ( mgr.try_flip( i ) )
      {
        if ( i < n )
        {
          L( "[i]   flip input " << i );
        }
        else
        {
          L( "[i]   flip output" );
        }
        improvement = true;
      }
    }

    /* permute inputs */
    for ( auto d = 1u; d < n - 1; ++d )
    {
      for ( auto i = 0u; i < n - d; ++i )
      {
        auto j = i + d;

        if ( mgr.try_swap( i, j ) )
        {
          L( "[i]   swap inputs " << i << " and " << j );
          improvement = true;
        }
      }
    }
  }

  fill_statistics( mgr, statistics );
}

template<typename Manager>
void aig_npn_canonization_sifting_generic( const aig_graph& aig, boost::dynamic_bitset<>& phase, std::vector<unsigned>& perm,
                                           const properties::ptr& settings,
                                           const properties::ptr& statistics )
{
  /* settings */
  const auto verbose = get( settings, "verbose", false );

  /* timing */
  properties_timer t( statistics );

  const auto& info = aig_info( aig );

  assert( info.outputs.size() == 1u ); /* assume single output AIG */

  Manager mgr( aig, phase, perm, settings );

  auto improvement = true;
  auto round = 0u;
  auto forward = true;
  const auto n = info.inputs.size();

  if ( n < 2u )
  {
    fill_statistics( mgr, statistics );
    return;
  }

  /* non-inverted function */
  while ( improvement )
  {
    L( "[i] round " << ++round );

    improvement = false;

    for ( int i = forward ? 0 : n - 2; forward ? i < static_cast<int>( n - 1 ) : i >= 0; forward ? ++i : --i )
    {
      if ( mgr.try_sift( i ) )
      {
        improvement = true;
      }
    }

    forward = !forward;
  }

  auto best_perm = perm;
  auto best_phase = phase;

  /* inverted function */
  mgr.reset( false );

  round = 0u;
  improvement = true;
  forward = true;

  while ( improvement )
  {
    L( "[i] round " << ++round );

    improvement = false;

    for ( int i = forward ? 0 : n - 2; forward ? i < static_cast<int>( n - 1 ) : i >= 0; forward ? ++i : --i )
    {
      if ( mgr.try_sift( i ) )
      {
        improvement = true;
      }
    }

    forward = !forward;
  }

  mgr.try_explicit( best_perm, best_phase );

  fill_statistics( mgr, statistics );
}

void aig_npn_canonization_flip_swap( const aig_graph& aig, boost::dynamic_bitset<>& phase, std::vector<unsigned>& perm,
                                     const properties::ptr& settings,
                                     const properties::ptr& statistics )
{
  aig_npn_canonization_flip_swap_generic<aig_npn_canonization_manager>( aig, phase, perm, settings, statistics );
}

void aig_npn_canonization_flip_swap_shared_miter( const aig_graph& aig, boost::dynamic_bitset<>& phase, std::vector<unsigned>& perm,
                                                  const properties::ptr& settings,
                                                  const properties::ptr& statistics )
{
  aig_npn_canonization_flip_swap_generic<aig_npn_canonization_shared_miter_manager>( aig, phase, perm, settings, statistics );
}

void aig_npn_canonization_sifting( const aig_graph& aig, boost::dynamic_bitset<>& phase, std::vector<unsigned>& perm,
                                   const properties::ptr& settings,
                                   const properties::ptr& statistics )
{
  aig_npn_canonization_sifting_generic<aig_npn_canonization_manager>( aig, phase, perm, settings, statistics );
}

void aig_npn_canonization_sifting_shared_miter( const aig_graph& aig, boost::dynamic_bitset<>& phase, std::vector<unsigned>& perm,
                                                const properties::ptr& settings,
                                                const properties::ptr& statistics )
{
  aig_npn_canonization_sifting_generic<aig_npn_canonization_shared_miter_manager>( aig, phase, perm, settings, statistics );
}

aig_graph aig_npn_canonization( const aig_graph& aig,
                                const properties::ptr& settings,
                                const properties::ptr& statistics )
{
  /* settings */
  const auto verbose   = get( settings, "verbose",   false );
  const auto progress  = get( settings, "progress",  false );
  const auto miter     = get( settings, "miter",     0u );    /* 0: single, 1: shared */
  const auto heuristic = get( settings, "heuristic", 0u );    /* 0: flip-swap, 1: sifting */

  /* timining */
  properties_timer t( statistics );

  /* new AIG */
  const auto& info = aig_info( aig );
  aig_graph aig_new;
  aig_initialize( aig_new, info.model_name );

  /* create PIs */
  for ( const auto& input : info.inputs )
  {
    aig_create_pi( aig_new, info.node_names.at( input ) );
  }

  /* NPN for each output */
  null_stream ns;
  std::ostream null_out( &ns );
  boost::progress_display show_progress( info.outputs.size(), progress ? std::cout : null_out );

  auto anc_statistics = std::make_shared<properties>();
  auto lexsat_calls = 0ul;
  auto sat_calls = 0ul;
  auto sat_runtime = 0.0;
  auto miter_runtime = 0.0;
  auto encoding_runtime = 0.0;

  std::vector<unsigned> num_inputs;

  for ( const auto& output : index( info.outputs ) )
  {
    ++show_progress;

    L( "[i] NPN for output " << output.index );

    const auto cone_statistics = std::make_shared<properties>();
    const auto cone = aig_cone( aig, std::vector<unsigned>{ static_cast<unsigned>( output.index ) }, properties::ptr(), cone_statistics );
    const auto mapped_inputs = cone_statistics->get<boost::dynamic_bitset<>>( "mapped_inputs" );

    num_inputs.push_back( mapped_inputs.count() );

    aig_function new_f;

    if ( mapped_inputs.none() )
    {
      L( "[i] constant case" );
      const auto& cone_info = aig_info( cone );
      assert( cone_info.outputs[0u].first.node == 0u );
      new_f = aig_get_constant( aig_new, false );
    }
    else
    {
      boost::dynamic_bitset<> phase;
      std::vector<unsigned> perm;

      if ( heuristic == 0u )
      {
        if ( miter == 0u )
        {
          aig_npn_canonization_flip_swap( cone, phase, perm, settings, anc_statistics );
        }
        else
        {
          aig_npn_canonization_flip_swap_shared_miter( cone, phase, perm, settings, anc_statistics );
        }
      }
      else
      {
        if ( miter == 0u )
        {
          aig_npn_canonization_sifting( cone, phase, perm, settings, anc_statistics );
        }
        else
        {
          aig_npn_canonization_sifting_shared_miter( cone, phase, perm, settings, anc_statistics );
        }
      }

      lexsat_calls += anc_statistics->get<unsigned long>( "lexsat_calls" );
      sat_calls += anc_statistics->get<unsigned long>( "sat_calls" );
      sat_runtime += anc_statistics->get<double>( "sat_runtime" );
      miter_runtime += anc_statistics->get<double>( "miter_runtime" );
      encoding_runtime += anc_statistics->get<double>( "encoding_runtime" );

      L( "[i] found NPN class with perm " << any_join( perm, " " ) << " and phase " << to_string( phase ) );
      L( "[i] mapped inputs " << mapped_inputs );

      const auto perm_t = translate_permutation( perm, mapped_inputs );
      const auto phase_t = translate_phase( phase, mapped_inputs );

      L( "[i] after mapping, phase " << to_string( phase_t ) );
      if ( verbose )
      {
        for ( const auto& p : perm_t )
        {
          std::cout << "[i] perm_t[" << p.first << "] = " << p.second << std::endl;
        }
      }

      new_f = make_function( simulate_aig_function( aig, output.value.first, strash_simulator( aig_new, 0u, perm_t, phase_t ) ), phase[phase.size() - 1u] );
    }
    aig_create_po( aig_new, new_f, output.value.second );
  }

  set( statistics, "lexsat_calls", lexsat_calls );
  set( statistics, "sat_calls", sat_calls );
  set( statistics, "sat_runtime", sat_runtime );
  set( statistics, "miter_runtime", miter_runtime );
  set( statistics, "encoding_runtime", encoding_runtime );
  set( statistics, "num_inputs", num_inputs );

  return aig_new;
}

}


/* SOME OLD CODE: creates the updated AIG from phase and perm */

  // /* propagate constants and reorder */
  // boost::dynamic_bitset<> invert( n );
  // std::map<unsigned, unsigned> reorder;
  // for ( auto i = 0u; i < n; ++i )
  // {
  //   invert[i] = phase[i];
  //   reorder.insert( {i, perm[i]} );
  // }

  // const auto s_settings = std::make_shared<properties>();
  // s_settings->set( "reorder", reorder );
  // s_settings->set( "invert",  invert );
  // auto aig_npn = strash( aig, s_settings );

  // if ( phase[n] )
  // {
  //   aig_info( aig_npn ).outputs[0].first.complemented ^= 1;
  // }

  // return aig_npn;

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
