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

#include "dbs.hpp"

#include <sstream>
#include <vector>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/min.hpp>
#include <boost/format.hpp>
#include <boost/range/algorithm.hpp>

#include <alice/rules.hpp>

#include <core/utils/program_options.hpp>
#include <core/utils/string_utils.hpp>
#include <classical/optimization/esop_minimization.hpp>
#include <classical/optimization/exorcism_minimization.hpp>
#include <reversible/circuit.hpp>
#include <reversible/rcbdd.hpp>
#include <reversible/truth_table.hpp>
#include <reversible/cli/stores.hpp>
#include <reversible/synthesis/rcbdd_synthesis.hpp>
#include <reversible/synthesis/young_subgroup_synthesis.hpp>

using namespace boost::program_options;

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

std::tuple<int, int, double> compute_stats( const std::vector<int>& sizes )
{
  using namespace boost::accumulators;

  accumulator_set<double, stats<tag::mean, tag::min, tag::max>> acc;
  acc = boost::for_each( sizes, acc );

  return std::make_tuple( min( acc ), max( acc ), mean( acc ) );
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

dbs_command::dbs_command( const environment::ptr& env )
  : cirkit_command( env, "Decomposition based synthesis", "[A. De Vos, Y. Van Rentergem, Adv. Math. Commun. 2 (2008), 183-200]\n[M. Soeken, L. Tague, G.W. Dueck, R. Drechsler, J. of Symb. Comp. 73 (2016), 1-26]" )
{
  opts.add_options()
    ( "symbolic,s",                                            "use symbolic variant (works on RCBDDs)" )
    ( "esop_minimizer", value_with_default( &esop_minimizer ), "ESOP minizer (0: built-in, 1: exorcism); only with symbolic approach" )
    ( "new,n",                                                 "add a new entry to the store; if not set, the current entry is overriden" )
    ;

  options_description tt_options( "Options for the truth table variant" );
  tt_options.add_options()
    ( "ordering",       value( &ordering ),                    "complete variable ordering (space separated, only for truth table variant)" )
    ;

  options_description symb_options( "Options for the symbolic variant" );
  symb_options.add_options()
    ( "mode",           value_with_default( &mode ),           "mode (0: default, 1: swap, 2: hamming)" )
    ( "progress,p",                                            "show progress" )
    ;

  opts.add( tt_options );
  opts.add( symb_options );

  be_verbose();
}

command::rules_t dbs_command::validity_rules() const
{
  return {
    { [&]() { return !this->is_set( "symbolic" ) || env->store<rcbdd>().current_index() >= 0; }, "symbolic method require RCBDD in store" },
    { [&]() { return this->is_set( "symbolic" ) || env->store<binary_truth_table>().current_index() >= 0; }, "no truth table in store" }
  };
}

bool dbs_command::execute()
{
  auto& circuits = env->store<circuit>();
  auto& rcbdds   = env->store<rcbdd>();
  auto& specs    = env->store<binary_truth_table>();

  auto settings = make_settings();

  if ( circuits.empty() || is_set( "new" ) )
  {
    circuits.extend();
  }

  circuit circ;

  auto esopmin_settings = std::make_shared<properties>();
  esopmin_settings->set( "verbose", is_verbose() );
  settings->set( "esopmin", esop_minimizer ? dd_based_exorcism_minimization_func( esopmin_settings ) : dd_based_esop_minimization_func( esopmin_settings ) );

  if ( is_set( "symbolic" ) )
  {
    settings->set( "mode", mode );
    settings->set( "progress", is_set( "progress" ) );

    rcbdd_synthesis( circ, rcbdds.current(), settings, statistics );
  }
  else
  {
    if ( !ordering.empty() )
    {
      std::vector<unsigned> vuordering;
      parse_string_list( vuordering, ordering );
      settings->set( "ordering", vuordering );
    }

    young_subgroup_synthesis( circ, specs.current(), settings, statistics );
  }

  circuits.current() = circ;

  std::cout << boost::format( "[i] run-time: %.2f secs" ) % statistics->get<double>( "runtime" ) << std::endl;

  return true;
}

command::log_opt_t dbs_command::log() const
{
  log_map_t map = {
    { "runtime", statistics->get<double>( "runtime" ) }
  };

  if ( is_set( "symbolic" ) )
  {
    const std::vector<int>& node_count = statistics->get<std::vector<int>>( "node_count" );
    auto stats = compute_stats( node_count );
    map["node_count"] = node_count;
    map["min_size"] = std::get<0>( stats );
    map["max_size"] = std::get<1>( stats );
    map["mean_size"] = std::get<2>( stats );

    std::stringstream ss;
    ss << statistics->get<unsigned long long>( "access" );
    map["access"] = ss.str();
  }

  return log_opt_t( map );
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
