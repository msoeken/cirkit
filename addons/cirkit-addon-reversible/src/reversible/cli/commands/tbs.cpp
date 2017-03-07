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

#include "tbs.hpp"

#include <alice/rules.hpp>

#include <core/utils/program_options.hpp>
#include <classical/aig.hpp>
#include <classical/cli/stores.hpp>
#include <reversible/circuit.hpp>
#include <reversible/rcbdd.hpp>
#include <reversible/truth_table.hpp>
#include <reversible/cli/stores.hpp>
#include <reversible/synthesis/symbolic_transformation_based_synthesis.hpp>
#include <reversible/synthesis/transformation_based_synthesis.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

tbs_command::tbs_command( const environment::ptr& env )
  : cirkit_command( env, "Transformation based synthesis" )
{
  add_new_option();
  be_verbose();

  boost::program_options::options_description tt_opts( "Explicit truth table based" );
  tt_opts.add_options()
    ( "bidirectional",    value_with_default( &bidirectional ), "bidirectional synthesis" )
    ( "fredkin,f",                                              "use Fredkin gates" )
    ( "fredkin_lookback",                                       "optimized Fredkin gate insertation (only with `fredkin' enabled)" )
    ;

  boost::program_options::options_description bdd_opts( "Symbolic BDD based" );
  bdd_opts.add_options()
    ( "bdd,b", "use symbolic BDD-based variant (works on RCBDDs)" )
    ;

  boost::program_options::options_description sat_opts( "Symbolic SAT based" );
  sat_opts.add_options()
    ( "sat,s",           "use symbolic SAT-based variant (works on RCBDDS (default), circuits (-c), and AIGs (-a))" )
    ( "circuit,c",       "use circuit as input" )
    ( "aig,a",           "use AIG as input" )
    ( "cnf_from_aig",    "create initial CNF from AIG instead of BDD (if input is RCBDD)" )
    ( "optimize_aig",    "if CNF is from AIG, then optimize AIG before encoding (if input is RCBDD)" )
    ( "sorting_network", "use sorting network instead of cardinality constraints (if input is RCBDD)" )
    ( "inc_optimize",    "incrementally optimize after each Hamming weight (if input is RCBDD)" )
    ( "all_assumptions", "use all assumptions for the SAT call" )
    ;

  opts.add( tt_opts );
  opts.add( bdd_opts );
  opts.add( sat_opts );
}

command::rules_t tbs_command::validity_rules() const
{
  return {
    { [&]() { return !this->is_set( "bdd" ) || env->store<rcbdd>().current_index() >= 0; }, "symbolid BDD method requires RCBDD in store" },
    { [&]() { return !this->is_set( "sat" ) || env->store<rcbdd>().current_index() >= 0 || env->store<circuit>().current_index() >= 0 || env->store<aig_graph>().current_index() >= 0; }, "symbolid SAT method requires RCBDDor circuit in store" },
    { [&]() { return this->is_set( "bdd" ) || this->is_set( "sat" ) || env->store<binary_truth_table>().current_index() >= 0; }, "no truth table in store" },
    { [&]() { return static_cast<int>( this->is_set( "bdd" ) ) + static_cast<int>( this->is_set( "sat" ) ) <= 1; }, "options bdd and sat cannot be set at the same time" }
  };
}

bool tbs_command::execute()
{
  auto& circuits = env->store<circuit>();
  auto& rcbdds   = env->store<rcbdd>();
  auto& specs    = env->store<binary_truth_table>();

  auto settings = make_settings();

  circuit circ;

  if ( is_set( "bdd" ) )
  {
    symbolic_transformation_based_synthesis( circ, rcbdds.current(), settings, statistics );
    std::cout << boost::format( "[i] adjusted assignments: %d" ) % statistics->get<unsigned>( "assignment_count" ) << std::endl;
  }
  else if ( is_set( "sat" ) )
  {
    settings->set( "cnf_from_aig", is_set( "cnf_from_aig" ) );
    settings->set( "optimize_aig", is_set( "optimize_aig" ) );
    settings->set( "all_assumptions", is_set( "all_assumptions" ) );
    if ( is_set( "circuit" ) )
    {
      symbolic_transformation_based_synthesis_sat( circ, circuits.current(), settings, statistics );
    }
    else if ( is_set( "aig" ) )
    {
      const auto& aigs = env->store<aig_graph>();
      symbolic_transformation_based_synthesis_sat( circ, aigs.current(), settings, statistics );
    }
    else if ( is_set( "inc_optimize" ) )
    {
      symbolic_transformation_based_synthesis_sat_incremental( circ, rcbdds.current(), settings, statistics );
    }
    else if ( is_set( "sorting_network" ) )
    {
      symbolic_transformation_based_synthesis_sat_sorting_network( circ, rcbdds.current(), settings, statistics );
    }
    else
    {
      symbolic_transformation_based_synthesis_sat( circ, rcbdds.current(), settings, statistics );
    }
    std::cout << boost::format( "[i] adjusted assignments: %d" ) % statistics->get<unsigned>( "assignment_count" ) << std::endl;
  }
  else
  {
    settings->set( "bidirectional",    bidirectional );
    settings->set( "fredkin",          is_set( "fredkin" ) );
    settings->set( "fredkin_lookback", is_set( "fredkin_lookback" ) );
    transformation_based_synthesis( circ, specs.current(), settings, statistics );
  }

  extend_if_new( circuits );

  circuits.current() = circ;

  print_runtime();
  if ( is_set( "sat" ) )
  {
    print_runtime( "solving_time", "SAT solving" );
  }

  return true;
}

command::log_opt_t tbs_command::log() const
{
  return log_opt_t({
      {"runtime", statistics->get<double>( "runtime" )},
      {"bdd", is_set( "bdd" )},
      {"sat", is_set( "sat" )},
      {"aig_from_cnf", is_set( "aig_from_cnf" )},
      {"optimize_aig", is_set( "optimize_aig" )},
      {"sorting_network", is_set( "sorting_network" )}
    });
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
