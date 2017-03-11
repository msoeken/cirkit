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

#include "unate.hpp"

#include <mutex>

#include <boost/assign/std/vector.hpp>
#include <boost/range/algorithm.hpp>

#include <core/utils/range_utils.hpp>
#include <core/utils/terminal.hpp>
#include <core/utils/thread_pool.hpp>
#include <core/utils/timer.hpp>
#include <classical/functions/aig_cone.hpp>
#include <classical/functions/simulate_aig.hpp>
#include <classical/functions/strash.hpp>
#include <classical/io/write_aiger.hpp>
#include <classical/utils/aig_utils.hpp>
#include <classical/abc/abc_api.hpp>
#include <classical/abc/functions/cirkit_to_gia.hpp>
#include <classical/sat/minisat.hpp>
#include <classical/sat/sat_solver.hpp>
#include <classical/sat/operations/logic.hpp>
#include <classical/sat/utils/add_aig.hpp>
#include <classical/sat/utils/add_aig_with_gia.hpp>
#include <classical/sat/utils/add_dimacs.hpp>

#define timer timer_class
#include <boost/progress.hpp>
#undef timer

#include <aig/gia/gia.h>

using namespace boost::assign;

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

class cofactor_miter_simulator : public aig_simulator<aig_function>
{
public:
  cofactor_miter_simulator( aig_graph& miter, const std::vector<aig_function>& pi_map )
    : miter( miter ),
      pi_map( pi_map )
  {
  }

  aig_function get_input( const aig_node& node, const std::string& name, unsigned pos, const aig_graph& aig ) const
  {
    return pi_map[pos];
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
  const std::vector<aig_function>& pi_map;
};

aig_graph create_cofactor_miter( const aig_graph& aig, unsigned input )
{
  const auto& info = aig_info( aig );

  aig_graph miter;
  aig_initialize( miter );

  std::vector<aig_function> c1_pis( info.inputs.size() );
  std::vector<aig_function> c2_pis( info.inputs.size() );

  for ( auto i = 0u; i < info.inputs.size(); ++i )
  {
    if ( i == input )
    {
      c1_pis[i] = aig_get_constant( miter, false );
      c2_pis[i] = aig_get_constant( miter, true );
    }
    else
    {
      const auto pi = aig_create_pi( miter, info.node_names.at( info.inputs[i] ) );
      c1_pis[i] = c2_pis[i] = pi;
    }
  }

  const auto out1 = simulate_aig( aig, cofactor_miter_simulator( miter, c1_pis ) );
  const auto out2 = simulate_aig( aig, cofactor_miter_simulator( miter, c2_pis ) );

  for ( const auto& output : info.outputs )
  {
    aig_create_po( miter, aig_create_xor( miter, out1.at( output.first ), out2.at( output.first ) ), output.second + "_dep" );
    aig_create_po( miter, aig_create_or( miter, out1.at( output.first ), !out2.at( output.first ) ), output.second + "_unate_neg" );
    aig_create_po( miter, aig_create_or( miter, !out1.at( output.first ), out2.at( output.first ) ), output.second + "_unate_pos" );
  }

  return miter;
}

boost::dynamic_bitset<> unateness_single_input( const aig_graph& aig, unsigned input )
{
  const auto miter = create_cofactor_miter( aig, input );
  const auto& info = aig_info( aig );
  const auto n = info.inputs.size();
  const auto m = info.outputs.size();

  /* create solver */
  auto solver = make_solver<minisat_solver>();
  solver_gen_model( solver, false );
  solver_execution_statistics stats;
  solver_result_t             sresult;

  std::vector<int> piids, poids;
  add_aig_with_gia( solver, miter, 1, piids, poids );

  assert( 3u * m == poids.size() );

  boost::dynamic_bitset<> result( ( m * n ) << 1u );
  auto pos = input << 1u;

  for ( auto j = 0u; j < m; ++j )
  {
    /* dependency */
    if ( solve( solver, stats, {poids[j * 3u]} ) == boost::none ) /* unsat */
    {
      result[pos] = 1; result[pos + 1u] = 1;
    }
    /* negative unate */
    else if ( solve( solver, stats, {-poids[j * 3u + 1u]} ) == boost::none )
    {
      result[pos] = 1; result[pos + 1u] = 0;
    }
    /* positive unate */
    else if ( solve( solver, stats, {-poids[j * 3u + 2u]} ) == boost::none )
    {
      result[pos] = 0; result[pos + 1u] = 1;
    }
    /* binate */
    else
    {
      result[pos] = 0; result[pos + 1u] = 0;
    }

    pos += ( n << 1u );
  }

  return result;
}

boost::dynamic_bitset<> unateness_single_output( const aig_graph& aig, unsigned output )
{
  /* create cone */
  const auto statistics = std::make_shared<properties>();
  const auto cone = aig_cone( aig, std::vector<unsigned>{output}, properties::ptr(), statistics );
  const auto mapped_inputs = statistics->get<boost::dynamic_bitset<>>( "mapped_inputs" );

  /* create miter */
  auto miter = strash( cone );
  strash( cone, miter );
  auto& info = aig_info( miter );

  assert( info.outputs.size() == 2u );

  /* connect outputs */
  auto o1 = info.outputs[0u].first;
  auto o2 = info.outputs[1u].first;
  info.outputs = {{aig_create_xor( miter, o1, o2 ), "dep"}, {aig_create_or( miter, !o1, o2 ), "unate"}};
  std::vector<std::pair<aig_function, std::string>> miter_outputs;

  /* connect inputs */
  const auto n = info.inputs.size() >> 1u;
  for ( auto i = 0u; i < n; ++i )
  {
    const auto& i1 = info.inputs[i];
    const auto& i2 = info.inputs[n + i];

    info.node_names[i2] = info.node_names[i1] + "_copy";

    aig_create_po( miter, !aig_create_xor( miter, {i1, false}, {i2, false} ), info.node_names[i1] + "_eq" );
  }

  /* create solver */
  auto solver = make_solver<minisat_solver>();
  solver_gen_model( solver, false );
  solver_execution_statistics stats;
  solver_result_t             sresult;

  std::vector<int> piids, poids;
  add_aig_with_gia( solver, miter, 1, piids, poids );

  boost::dynamic_bitset<> result( mapped_inputs.size() << 1u );
  result.set();
  boost::dynamic_bitset<>::size_type pos = boost::dynamic_bitset<>::npos;

  /* prepare assumptions */
  std::vector<int> assumptions( poids.begin() + 2u, poids.end() );

  for ( auto i = 0u; i < n; ++i )
  {
    pos = ( i == 0 ) ? mapped_inputs.find_first() : mapped_inputs.find_next( pos );

    assert( pos != boost::dynamic_bitset<>::npos );

    assumptions[i] *= -1;                  /* input i should be different */

    /* check for support */
    assumptions += poids[0u];              /* force XOR gate to be 1 */

    sresult = solve( solver, stats, assumptions );
    //sat_runtime += stats.runtime;
    if ( sresult == boost::none ) /* unsat */
    {
      result[pos << 1u] = 1; result[( pos << 1u ) + 1u] = 1;
      goto reset_assumptions;
    }

    /* check for negative unate */
    assumptions.back() = -poids[1u];       /* force OR gate to be 0 */
    assumptions += piids[i],-piids[n + i]; /* input i should be (1,0) */

    sresult = solve( solver, stats, assumptions );
    //sat_runtime += stats.runtime;
    if ( sresult == boost::none ) /* unsat */
    {
      result[pos << 1u] = 1; result[( pos << 1u ) + 1u] = 0;
      goto reset_assumptions;
    }

    /* check for positive unate */
    assumptions[n + 1] *= -1;              /* input i should be (0,1) */
    assumptions[n + 2] *= -1;

    sresult = solve( solver, stats, assumptions );
    //sat_runtime += stats.runtime;
    if ( sresult == boost::none ) /* unsat */
    {
      result[pos << 1u] = 0; result[( pos << 1u ) + 1u] = 1;
      goto reset_assumptions;
    }

    result[pos << 1u] = 0; result[( pos << 1u ) + 1u] = 0; /* binate */

reset_assumptions:
    assumptions.resize( n );
    assumptions[i] *= -1;
  }

  assert( mapped_inputs.find_next( pos ) == boost::dynamic_bitset<>::npos );

  return result;
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

boost::dynamic_bitset<> unateness_naive( const aig_graph& aig,
                                         const properties::ptr& settings,
                                         const properties::ptr& statistics )
{
  /* settings */
  const auto progress = get( settings, "progress", false );

  /* timer */
  properties_timer t( statistics );

  /* info */
  const auto& info = aig_info( aig );

  /* create solver */
  auto solver = make_solver<minisat_solver>();
  solver_gen_model( solver, false );
  solver_execution_statistics stats;
  auto sid = 1;

  /* build miter */
  std::vector<int> piids1, piids2, poids1, poids2;
  sid = add_aig( solver, aig, sid, piids1, poids1 );
  sid = add_aig( solver, aig, sid, piids2, poids2 );

  /* connect inputs */
  const auto n = info.inputs.size();
  std::vector<int> input_xnors( n );
  for ( auto i = 0u; i < n; ++i )
  {
    logic_xnor( solver, piids1[i], piids2[i], sid );
    input_xnors[i] = sid++;
  }

  /* connect outputs */
  const auto m = info.outputs.size();
  std::vector<int> output_xors( n ), output_ors( n );
  for ( auto j = 0u; j < m; ++j )
  {
    logic_xor( solver, poids1[j], poids2[j], sid );
    output_xors[j] = sid++;

    logic_or( solver, -poids1[j], poids2[j], sid );
    output_ors[j] = sid++;
  }

  /* iterate over output/input pairs */
  null_stream ns;
  std::ostream null_out( &ns );
  boost::progress_display show_progress( m * n, progress ? std::cout : null_out );

  boost::dynamic_bitset<> result( ( m * n ) << 1u );
  auto pos = 0u;
  for ( auto j = 0u; j < m; ++j )
  {
    for ( auto i = 0u; i < n; ++i )
    {
      ++show_progress;

      /* prepare assumptions */
      std::vector<int> assumptions( n - 1u );
      boost::remove_copy( input_xnors, assumptions.begin(), input_xnors[i] );

      /* assume different values for x_i */
      assumptions += piids1[i],-piids2[i];

      /* check for support */
      assumptions += output_xors[j];

      if ( solve( solver, stats, assumptions ) == boost::none ) /* unsat */
      {
        result[pos++] = 1; result[pos++] = 1;
        continue;
      }

      /* check for negative unate */
      assumptions.back() = -output_ors[j];

      if ( solve( solver, stats, assumptions ) == boost::none ) /* unsat */
      {
        result[pos++] = 1; result[pos++] = 0;
        continue;
      }

      /* check for positive unate */
      assumptions[n - 1u] *= -1;
      assumptions[n] *= -1;

      if ( solve( solver, stats, assumptions ) == boost::none ) /* unsat */
      {
        result[pos++] = 0; result[pos++] = 1;
        continue;
      }

      result[pos++] = 0; result[pos++] = 0;
    }
  }

  assert( pos == ( m * n ) << 1u );

  return result;
}

boost::dynamic_bitset<> unateness_split( const aig_graph& aig,
                                         const properties::ptr& settings,
                                         const properties::ptr& statistics )
{
  /* settings */
  const auto progress = get( settings, "progress", false );

  /* timer */
  properties_timer t( statistics );

  const auto& info = aig_info( aig );
  const auto n = info.inputs.size();
  const auto m = info.outputs.size();

  boost::dynamic_bitset<> result( ( m * n ) << 1u );
  auto pos = 0u;

  /* progress */
  null_stream ns;
  std::ostream null_out( &ns );
  boost::progress_display show_progress( m, progress ? std::cout : null_out );

  for ( auto j = 0u; j < m; ++j )
  {
    if ( progress ) { ++show_progress; }

    const auto cresult = unateness_single_output( aig, j );

    for ( auto b = 0u; b < cresult.size(); ++b )
    {
      result[pos++] = cresult[b];
    }
  }

  assert( pos == ( m * n ) << 1u );

  return result;
}

boost::dynamic_bitset<> unateness_split_parallel( const aig_graph& aig,
                                                  const properties::ptr& settings,
                                                  const properties::ptr& statistics )
{
  /* timer */
  properties_timer t( statistics );

  const auto& info = aig_info( aig );
  const auto n = info.inputs.size();
  const auto m = info.outputs.size();

  boost::dynamic_bitset<> result( ( m * n ) << 1u );

  std::mutex result_mutex;
  const auto thread = [&]( unsigned j ) {
    const auto cresult = unateness_single_output( aig, j );

    result_mutex.lock();
    auto pos = ( j * n ) << 1u;
    for ( auto b = 0u; b < cresult.size(); ++b )
    {
      result[pos++] = cresult[b];
    }
    result_mutex.unlock();
  };

  {
    thread_pool pool;

    for ( auto j = 0u; j < m; ++j )
    {
      pool.enqueue( thread, j );
    }
  }

  return result;
}

boost::dynamic_bitset<> unateness_split_inputs_parallel( const aig_graph& aig,
                                                         const properties::ptr& settings,
                                                         const properties::ptr& statistics )
{
  /* timer */
  properties_timer t( statistics );

  const auto& info = aig_info( aig );
  const auto n = info.inputs.size();
  const auto m = info.outputs.size();

  boost::dynamic_bitset<> result( ( m * n ) << 1u );

  std::mutex result_mutex;
  const auto thread = [&]( unsigned i ) {
    const auto cresult = unateness_single_input( aig, i );

    result_mutex.lock();
    result |= cresult;
    result_mutex.unlock();
  };

  {
    thread_pool pool;

    for ( auto i = 0u; i < n; ++i )
    {
      pool.enqueue( thread, i );
    }
  }

  return result;
}

boost::dynamic_bitset<> unateness( const aig_graph& aig,
                                   const properties::ptr& settings,
                                   const properties::ptr& statistics )
{
  /* settings */
  const auto skiplist = get( settings, "skiplist", true );
  const auto progress = get( settings, "progress", false );

  /* timer */
  properties_timer t( statistics );
  auto sat_runtime = 0.0;

  /* create miter */
  auto miter = strash( aig );
  strash( aig, miter );
  auto& info = aig_info( miter );

  /* connect outputs */
  std::vector<std::pair<aig_function, std::string>> miter_outputs;
  const auto m = info.outputs.size() >> 1u;
  for ( auto i = 0u; i < m; ++i )
  {
    auto o1 = info.outputs[i].first;
    auto o2 = info.outputs[m + i].first;
    const auto& name = info.outputs[i].second;

    miter_outputs.push_back( {aig_create_xor( miter, o1, o2 ), name + "_dep"} );
    miter_outputs.push_back( {aig_create_or( miter, !o1, o2 ), name + "_unate"} );
  }
  info.outputs = miter_outputs;

  /* connect inputs */
  const auto n = info.inputs.size() >> 1u;
  for ( auto i = 0u; i < n; ++i )
  {
    const auto& i1 = info.inputs[i];
    const auto& i2 = info.inputs[n + i];

    info.node_names[i2] = info.node_names[i1] + "_copy";

    aig_create_po( miter, !aig_create_xor( miter, {i1, false}, {i2, false} ), info.node_names[i1] + "_eq" );
  }

  /* create solver */
  auto solver = make_solver<minisat_solver>();
  solver_gen_model( solver, false );
  solver_execution_statistics stats;
  solver_result_t             sresult;

  std::vector<int> piids, poids;
  add_aig_with_gia( solver, miter, 1, piids, poids );

  boost::dynamic_bitset<> result( ( m * n ) << 1u );
  auto pos = 0u;

  /* progress */
  null_stream ns;
  std::ostream null_out( &ns );
  boost::progress_display show_progress( m * n, progress ? std::cout : null_out );

  /* skip list */
  boost::dynamic_bitset<> can_skip( m * n );

  /* prepare assumptions */
  std::vector<int> assumptions( poids.begin() + ( m << 1u ), poids.end() );

  for ( auto j = 0u; j < m; ++j )
  {
    for ( auto i = 0u; i < n; ++i )
    {
      if ( progress ) { ++show_progress; }

      assumptions[i] *= -1;                  /* input i should be different */

      /* check for support */
      assumptions += poids[j << 1u];         /* force XOR gate to be 1 */

      if ( !skiplist || !can_skip[j * n + i] )
      {
        solver_gen_model( solver, skiplist );
        sresult = solve( solver, stats, assumptions );
        sat_runtime += stats.runtime;
        solver_gen_model( solver, false );
        if ( sresult == boost::none ) /* unsat */
        {
          result[pos++] = 1; result[pos++] = 1;
          goto reset_assumptions;
        }
        else if ( skiplist)
        {
          /* check for other depedent inputs */
          for ( auto j2 = j + 1; j2 < m; ++j2 )
          {
            if ( sresult->first[poids[ j2 << 1u ] - 1u] )
            {
              can_skip.set( j2 * n + i );
            }
          }
        }
      }

      /* check for negative unate */
      ++assumptions.back() *= -1;            /* force OR gate to be 0 */
      assumptions += piids[i],-piids[n + i]; /* input i should be (1,0) */

      sresult = solve( solver, stats, assumptions );
      sat_runtime += stats.runtime;
      if ( sresult == boost::none ) /* unsat */
      {
        result[pos++] = 1; result[pos++] = 0;
        goto reset_assumptions;
      }

      /* check for positive unate */
      assumptions[n + 1] *= -1;              /* input i should be (0,1) */
      assumptions[n + 2] *= -1;

      sresult = solve( solver, stats, assumptions );
      sat_runtime += stats.runtime;
      if ( sresult == boost::none ) /* unsat */
      {
        result[pos++] = 0; result[pos++] = 1;
        goto reset_assumptions;
      }

      //result[pos++] = 0; result[pos++] = 0;
      pos += 2u;

reset_assumptions:
      assumptions.resize( n );
      assumptions[i] *= -1;
    }
  }

  assert( pos == ( m * n ) << 1u );

  set( statistics, "sat_runtime", sat_runtime );

  return result;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
