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

#include "symbolic_transformation_based_synthesis.hpp"

#include <fstream>
#include <iostream>

#include <boost/assign/std/vector.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext/iota.hpp>

#include <core/utils/bdd_utils.hpp>
#include <core/utils/bitset_utils.hpp>
#include <core/utils/range_utils.hpp>
#include <core/utils/timer.hpp>

#include <classical/aig.hpp>
#include <classical/abc/utils/abc_run_command.hpp>
#include <classical/functions/strash.hpp>
#include <classical/io/read_aiger.hpp>
#include <classical/io/write_aiger.hpp>
#include <classical/utils/aig_from_bdd.hpp>
#include <classical/sat/abcsat.hpp>
#include <classical/sat/minisat.hpp>
#include <classical/sat/sat_solver.hpp>
#include <classical/sat/operations/cardinality.hpp>
#include <classical/sat/operations/logic.hpp>
#include <classical/sat/utils/add_aig.hpp>
#include <classical/sat/utils/add_aig_with_gia.hpp>
#include <classical/sat/utils/add_bdd.hpp>

#include <reversible/functions/add_gates.hpp>
#include <reversible/functions/copy_metadata.hpp>
#include <reversible/io/print_circuit.hpp>

/* statistics logger */
//#define STBS_STAT_LOGGER

#ifdef STBS_STAT_LOGGER
#define SL(x) x
#else
#define SL(x)
#endif

using namespace boost::assign;

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

BDD at_least_one_different( const rcbdd& cf )
{
  auto f = cf.manager().bddZero();

  for ( auto i = 0u; i < cf.num_vars(); ++i )
  {
    f |= cf.x( i ) ^ cf.y( i );
  }

  return f;
}

void assign_sets( const char * cube, unsigned j, std::vector<unsigned>& i10, std::vector<unsigned>& i01, std::vector<unsigned>& x1, std::vector<unsigned>& y1 )
{
  const auto cx = cube[3u * j];
  const auto cy = cube[3u * j + 1u];

  assert( cx == 0 || cx == 1 );
  assert( cy == 0 || cy == 1 );

  if ( cx == 1 )
  {
    x1 += j;
    if ( cy == 0 ) { i10 += j; }
  }

  if ( cy == 1 )
  {
    y1 += j;
    if ( cx == 0 ) { i01 += j; }
  }
}

template<typename S>
int difference_clauses( S& solver, const std::vector<int>& xs, const std::vector<int>& ys, int sid )
{
  std::vector<int> xorids( xs.size() );
  boost::iota( xorids, sid );
  sid += xs.size();

  for ( auto i = 0u; i < xs.size(); ++i )
  {
    logic_xor( solver, xs[i], ys[i], xorids[i] );
  }

  auto orid = sid;

  logic_or( solver, xorids, orid );

  add_clause( solver )( {orid + 1, orid} ); /* first literal is assumption */

  return orid + 1;
}

template<typename S>
int difference_clauses_plaisted_greenbaum( S& solver, const std::vector<int>& xs, const std::vector<int>& ys, int sid )
{
  std::vector<int> xorids( xs.size() );
  boost::iota( xorids, sid );
  sid += xs.size();

  for ( auto i = 0u; i < xs.size(); ++i )
  {
    add_clause( solver )( {xs[i], ys[i], -xorids[i]} );
    add_clause( solver )( {-xs[i], -ys[i], -xorids[i]} );
  }

  xorids.push_back( sid );
  add_clause( solver )( xorids );
  return sid;

  // auto orid = sid;

  // logic_or( solver, xorids, orid );

  // add_clause( solver )( {orid + 1, orid} );

  // return orid + 1;
}

template<typename S>
int get_controls( S& solver, const std::vector<unsigned>& controls, const std::vector<int>& xs, int one_literal, int& sid )
{
  if ( controls.empty() ) { return one_literal; }
  if ( controls.size() == 1u ) { return xs[controls.front()]; }
  else {
    std::vector<int> xsc( controls.size() );
    for ( auto i = 0u; i < controls.size(); ++i )
    {
      xsc[i] = xs[controls[i]];
    }

    logic_and( solver, xsc, sid++ );
    return sid - 1;
  }
}

template<typename S>
int get_controls_neg( S& solver, const gate::control_container& controls, const std::vector<int>& lines, int one_literal, int& sid )
{
  if ( controls.empty() ) { return one_literal; }
  if ( controls.size() == 1u ) { return controls.front().polarity() ? lines[controls.front().line()] : -lines[controls.front().line()]; }
  else {
    std::vector<int> xsc( controls.size() );
    for ( auto i = 0u; i < controls.size(); ++i )
    {
      xsc[i] = controls[i].polarity() ? lines[controls[i].line()] : -lines[controls[i].line()];
    }

    logic_and( solver, xsc, sid++ );
    return sid - 1;
  }
}

template<typename S>
void simulate( S& solver, const std::vector<int>& xs, const std::vector<int>& ys, const boost::optional<int>& diff = boost::none )
{
  boost::dynamic_bitset<> b( xs.size() );

  do
  {
    std::vector<int> assumptions;

    for ( auto i = 0u; i < xs.size(); ++i )
    {
      assumptions += ( b[i] ? xs[i] : -xs[i] );
    }

    if ( diff != boost::none )
    {
      assumptions += *diff;
    }

    solver_execution_statistics sstats;
    auto result = solve( solver, sstats, assumptions );

    if ( result != boost::none )
    {
      for ( auto i = 0u; i < xs.size(); ++i )
      {
        std::cout << result->first[xs[i] - 1];
      }
      std::cout << " ";
      for ( auto i = 0u; i < xs.size(); ++i )
      {
        std::cout << result->first[ys[i] - 1];
      }
      std::cout << std::endl;
    }
    else if ( diff == boost::none )
    {
      std::cout << "no assignment for " << b << std::endl;
    }

    inc( b );
  } while ( b.any() );
}

void extract_gate_information( const boost::dynamic_bitset<>& result,
                               const std::vector<int>& xs, const std::vector<int>& ys,
                               std::vector<unsigned>& i10, std::vector<unsigned>& i01,
                               std::vector<unsigned>& x1, std::vector<unsigned>& y1 )
{
  for ( auto i = 0u; i < xs.size(); ++i )
  {
    const auto cx = result[xs[i] - 1];
    const auto cy = result[ys[i] - 1];

    if ( cx == 1 )
    {
      x1 += i;
      if ( cy == 0 ) { i10 += i; }
    }

    if ( cy == 1 )
    {
      y1 += i;
      if ( cx == 0 ) { i01 += i; }
    }
  }
}

void prepend_multi_target_toffoli( circuit& circ, const std::vector<unsigned>& controls, const std::vector<unsigned>& targets, bool copy_target = true )
{
  if ( targets.empty() ) return;

  if ( copy_target )
  {
    for ( auto i = 1u; i < targets.size(); ++i )
    {
      prepend_cnot( circ, targets[0], targets[i] );
    }
    prepend_toffoli( circ, controls, targets[0] );
    for ( auto i = 1u; i < targets.size(); ++i )
    {
      prepend_cnot( circ, targets[0], targets[i] );
    }
  }
  else
  {
    for ( auto t : targets )
    {
      prepend_toffoli( circ, controls, t );
    }
  }
}

void add_gates_to_circuit( circuit& circ,
                           const std::vector<unsigned>& i10, const std::vector<unsigned>& i01,
                           const std::vector<unsigned>& x1, const std::vector<unsigned>& y1 )
{
  /* add gates to circuit */
  prepend_multi_target_toffoli( circ, y1, i10, true );
  prepend_multi_target_toffoli( circ, x1, i01, true );
}

template<typename Solver>
int create_initial_cnf( Solver& solver, const rcbdd& cf, int sid, std::vector<int>& xs, std::vector<int>& ys, int& one_literal, bool cnf_from_aig, bool optimize_aig )
{
  if ( cnf_from_aig )
  {
    aig_graph aig, new_aig;
    aig_initialize( aig );
    aig_create_po( aig, aig_from_bdd( aig, cf.chi() ), "f" );

    std::vector<int> piids, poids;

    if ( optimize_aig )
    {
      new_aig = abc_run_command( aig, "&put; balance; rewrite; refactor; balance; rewrite; rewrite -z; balance; refactor -z; rewrite -z; balance; &get" );
      sid = add_aig_with_gia( solver, new_aig, sid, piids, poids );
    }
    else
    {
      sid = add_aig_with_gia( solver, aig, sid, piids, poids );
    }

    add_clause( solver )( {poids[0]} );

    for ( auto i = 0u; i < xs.size(); ++i )
    {
      xs[i] = piids[3 * i];
      ys[i] = piids[3 * i + 1];
    }

    one_literal = sid++;
    add_clause( solver )( {one_literal} );
  }
  else
  {
    const auto f = cf.chi();

    std::map<DdNode*, int> node_to_var;
    const auto old_sid = sid;
    sid = add_bdd( solver, cf.manager().getManager(), {f.getNode()}, sid, node_to_var );

    auto chi_id = node_to_var.at( f.getRegularNode() );
    if ( Cudd_IsComplement( f.getNode() ) )
    {
      chi_id = -chi_id;
    }
    add_clause( solver )( {chi_id} );

    for ( auto i = 0u; i < xs.size(); ++i )
    {
      xs[i] = old_sid + 3 * i;
      ys[i] = old_sid + 3 * i + 1;
    }

    one_literal = old_sid + 3 * xs.size();
  }

  return sid;
}

template<typename Solver>
void synthesize_with_sat( Solver& solver, circuit& circ, unsigned n, std::vector<int>& xs, std::vector<int>& ys, int one_literal, int& sid, bool all_assumptions, bool verbose, const properties::ptr& statistics )
{
  auto assignment_count = 0u;
  auto solving_time     = 0.0;

  if ( verbose )
  {
    //std::cout << "[i] initial number of clauses: " << solver.solver->nClauses() << std::endl;
  }

  //cf.print_truth_table();
  //simulate( solver, xs, ys );

  /* for incremental solving */
  int assume_weight, assume_diff;

  solver_result_t result;
  solver_execution_statistics sstats;

  SL( const double sec = 1000000000.0L; )
  SL( std::ofstream log( "/tmp/stbs_sat.log", std::ofstream::out ); )
  SL( auto prevtime = t.elapsed().wall; )
  SL( log << boost::format( "0 0.00 %d" ) % solver.solver->nClauses() << std::endl; )

  std::vector<int> assume_weights;
  std::vector<int> assume_diffs;

  /* for each hamming weight */
  for ( auto k = 0u; k <= n; ++k )
  {
    if ( verbose )
    {
      std::cout << boost::format( "[i] k = %2d" ) % k;
      std::cout.flush();
    }

    assume_weight = sid++;
    solver_add_blocking_var( solver, assume_weight );
    sid = equals_sinz( solver, xs, k, sid );
    solver_clear_blocking_vars( solver );

    if ( all_assumptions )
    {
      if ( !assume_weights.empty() ) { assume_weights.back() *= -1; }
      assume_weights.push_back( -assume_weight );
    }

    auto count = 0u;
    auto local_runtime = 0.0;

    while ( true )
    {
      assume_diff = difference_clauses_plaisted_greenbaum( solver, xs, ys, sid );
      sid = assume_diff + 1;

      {
        increment_timer tim( &local_runtime );
        if ( all_assumptions )
        {
          if ( !assume_diffs.empty() ) { assume_diffs.back() *= -1; }
          assume_diffs.push_back( -assume_diff );

          std::vector<int> assumptions( assume_weights.size() + assume_diffs.size() );
          boost::copy( assume_weights, assumptions.begin() );
          boost::copy( assume_diffs, assumptions.begin() + assume_weights.size() );
          result = solve( solver, sstats, assumptions );
        }
        else
        {
          result = solve( solver, sstats, {-assume_weight, -assume_diff} );
          // std::vector<int> assumptions;
          // assumptions.push_back( -assume_diff );
          // for ( auto i = k; i < pow_n; ++i )
          // {
          //   assumptions.push_back( -ovars[i] );
          // }
          // result = solve( solver, sstats, assumptions );
        }
      }

      if ( result == boost::none ) { break; }
      ++count;

      std::vector<unsigned> i10, i01, x1, y1;
      extract_gate_information( result->first, xs, ys, i10, i01, x1, y1 );

      /* add gate constraints */
      if ( !i10.empty() )
      {
        auto controls = get_controls( solver, y1, ys, one_literal, sid );

        for ( auto target : i10 )
        {
          auto newx = sid++;
          logic_xor( solver, ys[target], controls, newx );
          ys[target] = newx;
        }
      }

      if ( !i01.empty() )
      {
        auto controls = get_controls( solver, x1, ys, one_literal, sid );

        for ( auto target : i01 )
        {
          auto newx = sid++;
          logic_xor( solver, ys[target], controls, newx );
          ys[target] = newx;
        }
      }

      //simulate( solver, xs, ys );

      /* add gates to circuit */
      add_gates_to_circuit( circ, i10, i01, x1, y1 );

      SL( const auto wall = t.elapsed().wall; )
      SL( log << boost::format( "%d %.2f %d" ) % ( assignment_count + count ) % ( ( wall - prevtime ) / sec ) % solver.solver->nClauses() << std::endl; )
      SL( prevtime = wall; )
    }

    solving_time += local_runtime;
    assignment_count += count;

    if ( verbose )
    {
      std::cout << boost::format( " assigns = %7d local = %9.2f avg = %4.2f total = %9.2f" ) % count % local_runtime % ( local_runtime / count ) % solving_time << std::endl;
    }
  }

  set( statistics, "assignment_count", assignment_count );
  set( statistics, "solving_time",     solving_time );
}

aig_graph synthesize_with_sat_incremental( unsigned k, const aig_graph& aig, circuit& circ, bool verbose, double& solving_time, unsigned& assignment_count )
{
  /* statistics */
  auto count         = 0u;
  auto local_runtime = 0.0;
  auto enc_runtime   = 0.0;

  const auto n = circ.lines();

  /* for SAT solving */
  solver_result_t result;
  solver_execution_statistics sstats;

  /* prepare new AIG (copy current and remove outputs) */
  auto aig_new = aig;
  auto& info_new = aig_info( aig_new );
  std::vector<aig_function> outputs_new( n );
  std::vector<std::string> names_old( n );
  for ( auto i = 0u; i < n; ++i )
  {
    outputs_new[i] = info_new.outputs[i + 1].first;
    names_old[i] = info_new.outputs[i + 1].second;
  }
  auto first_output = info_new.outputs.front().first;
  info_new.outputs.clear();
  aig_create_po( aig_new, first_output, "f" );

  /* create solver */
  auto solver = make_solver<minisat_solver>();

  std::vector<int> piids, poids, xs( n ), ys( n );
  int sid = 1;

  {
    reference_timer t( &enc_runtime );
    sid = add_aig/*_with_gia*/( solver, aig, sid, piids, poids );
  }

  add_clause( solver )( {poids[0]} );

  for ( auto i = 0u; i < n; ++i )
  {
    xs[i] = piids[3 * i];
    ys[i] = poids[i + 1];
  }

  /* add a literal that is always 1 */
  auto one_literal = sid++;
  add_clause( solver )( {one_literal} );

  /* for each hamming weight */
  if ( verbose )
  {
    std::cout << boost::format( "[i] k = %2d" ) % k;
    std::cout.flush();
  }

  sid = equals_sinz( solver, xs, k, sid );

  /* some helper function */
  const auto add_gates_to_aig = [&aig_new, &outputs_new]( const std::vector<unsigned>& controls, const std::vector<unsigned>& targets ) {
    /* update aig_new */
    if ( controls.empty() )
    {
      for ( auto target : targets )
      {
        outputs_new[target].complemented ^= 1;
      }
    }
    else
    {
      std::vector<aig_function> ops;
      for ( auto control : controls )
      {
        ops.push_back( outputs_new[control] );
      }
      for ( auto target : targets )
      {
        outputs_new[target] = aig_create_xor( aig_new, outputs_new[target], aig_create_nary_and( aig_new, ops ) );
      }
    }
  };

  while ( true )
  {
    const auto assume_diff = difference_clauses_plaisted_greenbaum( solver, xs, ys, sid );
    sid = assume_diff + 1;

    {
      increment_timer tim( &local_runtime );

      result = solve( solver, sstats, {-assume_diff} );
    }

    if ( result == boost::none ) { break; }
    ++count;

    std::vector<unsigned> i10, i01, x1, y1;
    extract_gate_information( result->first, xs, ys, i10, i01, x1, y1 );

    /* add gate constraints */
    if ( !i10.empty() )
    {
      auto controls = get_controls( solver, y1, ys, one_literal, sid );

      for ( auto target : i10 )
      {
        auto newx = sid++;
        logic_xor( solver, ys[target], controls, newx );
        ys[target] = newx;
      }

      /* update aig_new */
      add_gates_to_aig( y1, i10 );
    }

    if ( !i01.empty() )
    {
      auto controls = get_controls( solver, x1, ys, one_literal, sid );

      for ( auto target : i01 )
      {
        auto newx = sid++;
        logic_xor( solver, ys[target], controls, newx );
        ys[target] = newx;
      }

      /* update aig_new */
      add_gates_to_aig( x1, i01 );
    }

    /* add gates to circuit and new aig */
    add_gates_to_circuit( circ, i10, i01, x1, y1 );
  }

  assignment_count += count;
  solving_time += local_runtime;

  if ( verbose )
  {
    std::cout << boost::format( " assigns = %7d local = %9.2f avg = %4.2f total = %9.2f enc = %8.2f" ) % count % local_runtime % ( local_runtime / count ) % solving_time % enc_runtime;
  }

  for ( auto i = 0u; i < n; ++i )
  {
    aig_create_po( aig_new, outputs_new[i], names_old[i] );
  }

  return aig_new;
}

template<typename Solver>
void synthesize_with_sat_sorting_network( Solver& solver, circuit& circ, unsigned n, std::vector<int>& xs, std::vector<int>& ys, const std::vector<int>& card, int one_literal, int& sid, bool verbose, const properties::ptr& statistics )
{
  auto assignment_count = 0u;
  auto solving_time     = 0.0;

  /* for incremental solving */
  int assume_diff;

  solver_result_t result;
  solver_execution_statistics sstats;

  /* for each hamming weight */
  for ( auto k = 0u; k <= n; ++k )
  {
    if ( verbose )
    {
      std::cout << boost::format( "[i] k = %2d" ) % k;
      std::cout.flush();
    }

    auto count = 0u;
    auto local_runtime = 0.0;

    while ( true )
    {
      assume_diff = difference_clauses_plaisted_greenbaum( solver, xs, ys, sid );
      sid = assume_diff + 1;

      {
        increment_timer tim( &local_runtime );
        if ( k > 1 )
        {
          result = solve( solver, sstats, {-card[k], card[k - 1], -assume_diff} );
        }
        else
        {
          result = solve( solver, sstats, {-card[k], -assume_diff} );
        }
      }

      if ( result == boost::none ) { break; }
      ++count;

      std::vector<unsigned> i10, i01, x1, y1;
      extract_gate_information( result->first, xs, ys, i10, i01, x1, y1 );

      /* add gate constraints */
      if ( !i10.empty() )
      {
        auto controls = get_controls( solver, y1, ys, one_literal, sid );

        for ( auto target : i10 )
        {
          auto newx = sid++;
          logic_xor( solver, ys[target], controls, newx );
          ys[target] = newx;
        }
      }

      if ( !i01.empty() )
      {
        auto controls = get_controls( solver, x1, ys, one_literal, sid );

        for ( auto target : i01 )
        {
          auto newx = sid++;
          logic_xor( solver, ys[target], controls, newx );
          ys[target] = newx;
        }
      }

      /* add gates to circuit */
      add_gates_to_circuit( circ, i10, i01, x1, y1 );
    }

    solving_time += local_runtime;
    assignment_count += count;

    if ( verbose )
    {
      std::cout << boost::format( " assigns = %7d local = %9.2f avg = %4.2f total = %9.2f" ) % count % local_runtime % ( local_runtime / count ) % solving_time << std::endl;
    }
  }

  set( statistics, "assignment_count", assignment_count );
  set( statistics, "solving_time",     solving_time );
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

bool symbolic_transformation_based_synthesis( circuit& circ, const rcbdd& cf,
                                              const properties::ptr& settings,
                                              const properties::ptr& statistics )
{
  /* settings */
  const auto verbose = get( settings, "verbose", false );

  /* timer */
  properties_timer t( statistics );

  /* number of lines */
  const auto n = cf.num_vars();

  /* copy meta data */
  copy_meta_data( circ, cf );

  auto f = cf.chi();
  auto assignment_count = 0u;

  const auto diff = at_least_one_different( cf );

  char *cube = new char[3u * cf.num_vars()];

  SL( const double sec = 1000000000.0L; )
  SL( std::ofstream log( "/tmp/stbs_bdd.log", std::ofstream::out ); )
  SL( auto prevtime = t.elapsed().wall; )
  SL( log << boost::format( "0 0.00 %d" ) % f.nodeCount() << std::endl; )

  for ( auto i = 0u; i <= n; ++i )
  {
    if ( verbose )
    {
      std::cout << "[i] i = " << i << std::endl;
    }

    const auto card = make_eq( cf.manager(), cf.xs(), i );

    auto paths = f & diff & card;

    auto count = 0u;

    while ( paths != cf.manager().bddZero() )
    {
      paths.PickOneCube( cube );

      std::vector<unsigned> i10, i01, x1, y1;

      for ( auto j = 0u; j < n; ++j )
      {
        assign_sets( cube, j, i10, i01, x1, y1 );
      }

      add_gates_to_circuit( circ, i10, i01, x1, y1 );

      circuit tmp_circ( circ.lines() );

      for ( const auto& k : i10 )
      {
        auto& g = prepend_toffoli( tmp_circ, y1, k );
        f = cf.compose( f, cf.create_from_gate( g ) );
      }
      for ( const auto& k : i01 )
      {
        auto g = prepend_toffoli( tmp_circ, x1, k );
        f = cf.compose( f, cf.create_from_gate( g ) );
      }

      paths = f & diff & card;

      ++count;

      SL( const auto wall = t.elapsed().wall; )
      SL( log << boost::format( "%d %.2f %d" ) % ( assignment_count + count ) % ( ( wall - prevtime ) / sec ) % f.nodeCount() << std::endl; )
      SL( prevtime = wall; )
    }

    if ( verbose )
    {
      std::cout << "[i] #adjusted assignments: " << count << std::endl;
    }

    assignment_count += count;
  }

  delete[] cube;

  set( statistics, "assignment_count", assignment_count );

  return true;
}

bool symbolic_transformation_based_synthesis_sat( circuit& circ, const rcbdd& cf,
                                                  const properties::ptr& settings,
                                                  const properties::ptr& statistics )
{
  /* settings */
  const auto verbose         = get( settings, "verbose",         false );
  const auto cnf_from_aig    = get( settings, "cnf_from_aig",    false );
  const auto optimize_aig    = get( settings, "optimize_aig",    false );
  const auto all_assumptions = get( settings, "all_assumptions", false );

  /* timer */
  properties_timer t( statistics );

  /* number of lines */
  const auto n = cf.num_vars();

  /* copy meta data */
  copy_meta_data( circ, cf );

  auto solver = make_solver<minisat_solver>();

  int one_literal = -1;
  std::vector<int> xs( n ), ys( n );

  int sid = create_initial_cnf( solver, cf, 1, xs, ys, one_literal, cnf_from_aig, optimize_aig );
  synthesize_with_sat( solver, circ, n, xs, ys, one_literal, sid, all_assumptions, verbose, statistics );

  return true;
}

bool symbolic_transformation_based_synthesis_sat_incremental( circuit& circ, const rcbdd& cf,
                                                              const properties::ptr& settings,
                                                              const properties::ptr& statistics )
{
  /* settings */
  const auto verbose = get( settings, "verbose", false );

  /* timer */
  properties_timer t( statistics );

  /* number of lines */
  const auto n = cf.num_vars();

  /* copy meta data */
  copy_meta_data( circ, cf );

  auto f = cf.chi();

  /* create AIG from cf */
  aig_graph aig;
  aig_initialize( aig );
  aig_create_po( aig, aig_from_bdd( aig, cf.chi() ), "f" );
  auto info = aig_info( aig );

  /* expose y outputs */
  for ( auto i = 0u; i < n; ++i )
  {
    aig_create_po( aig, {info.inputs[ 3 * i + 1 ], false}, "y" + std::to_string( i ) );
  }

  auto solving_time = 0.0;
  auto assignment_count = 0u;

  auto paig = std::make_shared<aig_graph>( aig );

  for ( auto k = 0u; k <= n; ++k )
  {
    //const auto aig_opt = abc_run_command( *paig, "&put; balance; rewrite; refactor; balance; rewrite; rewrite -z; balance; refactor -z; rewrite -z; balance; &get" );

    auto aig_new = synthesize_with_sat_incremental( k, *paig, circ, verbose, solving_time, assignment_count );

    if ( verbose )
    {
      std::cout << boost::format( " node = %7d i/o = %d/%d" ) % num_vertices( aig_new ) % aig_info( aig_new ).inputs.size() % aig_info( aig_new ).outputs.size() << std::endl;
    }

    paig = std::make_shared<aig_graph>( aig_new );
  }

  set( statistics, "assignment_count", assignment_count );
  set( statistics, "solving_time",     solving_time );

  return true;
}

bool symbolic_transformation_based_synthesis_sat_sorting_network( circuit& circ, const rcbdd& cf,
                                                                  const properties::ptr& settings,
                                                                  const properties::ptr& statistics )
{
  /* settings */
  const auto cnf_from_aig = get( settings, "cnf_from_aig", false );
  const auto optimize_aig = get( settings, "optimize_aig", false );
  const auto verbose      = get( settings, "verbose",      false );

  /* timer */
  properties_timer t( statistics );

  /* number of lines */
  const auto n = cf.num_vars();

  /* copy meta data */
  copy_meta_data( circ, cf );

  auto f = cf.chi();

  auto solver = make_solver<minisat_solver>();

  /* first create sorter network */
  auto sid = 1;
  const auto log_n = static_cast<int>( ceil( log2( n ) ) );
  const auto pow_n = 1 << log_n;

  std::vector<int> ovars;
  sid = sat_pairwise_cardinality_network( solver, log_n, sid, &ovars );

  int one_literal = -1;
  std::vector<int> xs( n ), ys( n );
  sid = create_initial_cnf( solver, cf, sid, xs, ys, one_literal, cnf_from_aig, optimize_aig );

  for ( auto i = 0u; i < n; ++i )
  {
    equals( solver, xs[i], 1 + i ); /* 1 + i is input to sorter network */
  }

  synthesize_with_sat_sorting_network( solver, circ, n, xs, ys, ovars, one_literal, sid, verbose, statistics );

  return true;
}

bool symbolic_transformation_based_synthesis_sat( circuit& dest, const circuit& src,
                                                  const properties::ptr& settings,
                                                  const properties::ptr& statistics )
{
  /* settings */
  const auto verbose         = get( settings, "verbose",         false );
  const auto all_assumptions = get( settings, "all_assumptions", false );

  /* timer */
  properties_timer t( statistics );

  /* number of lines */
  const auto lines = src.lines();
  auto n = lines, n2 = lines;

  for ( const auto& c : src.constants() )
  {
    if ( (bool)c )
    {
      --n;
    }
  }
  for ( const auto& g : src.garbage() )
  {
    if ( g )
    {
      --n2;
    }
  }
  assert( n == n2 );

  /* copy meta data */
  copy_metadata( src, dest );

  auto solver = make_solver<minisat_solver>();

  auto sid = 1;
  std::vector<int> xs, ys, current( lines );

  int one_literal = sid++;
  add_clause( solver )( {one_literal} );

  for ( auto i = 0u; i < lines; ++i )
  {
    auto c = src.constants()[i];
    if ( (bool)c )
    {
      current[i] = *c ? one_literal : -one_literal;
    }
    else
    {
      xs.push_back( sid++ );
      current[i] = xs.back();
    }
  }

  for ( const auto& g : src )
  {
    auto controls = get_controls_neg( solver, g.controls(), current, one_literal, sid );
    assert( g.targets().size() == 1u );

    auto target = g.targets().front();
    auto newl = sid++;
    logic_xor( solver, current[target], controls, newl );
    current[target] = newl;
  }

  for ( auto i = 0u; i < lines; ++i )
  {
    if ( !src.garbage()[i] )
    {
      ys.push_back( current[i] );
    }
  }

  synthesize_with_sat( solver, dest, n, xs, ys, one_literal, sid, all_assumptions, verbose, statistics );

  return true;
}

bool symbolic_transformation_based_synthesis_sat( circuit& dest, const aig_graph& src,
                                                  const properties::ptr& settings,
                                                  const properties::ptr& statistics )
{
  /* settings */
  const auto verbose         = get( settings, "verbose",         false );
  const auto all_assumptions = get( settings, "all_assumptions", false );

  /* timer */
  properties_timer t( statistics );

  /* copy meta data */
  //copy_metadata( src, dest );

  auto solver = make_solver<minisat_solver>();

  /* create instance */
  std::vector<int> xs, ys;
  int sid = 1;
  sid = add_aig_with_gia( solver, src, sid, xs, ys );

  int one_literal = sid++;
  add_clause( solver )( {one_literal} );

  assert( xs.size() == ys.size() );
  auto n = xs.size();

  dest.set_lines( n );

  synthesize_with_sat( solver, dest, n, xs, ys, one_literal, sid, all_assumptions, verbose, statistics );

  return true;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
