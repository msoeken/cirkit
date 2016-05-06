/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2016  EPFL
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

#include <lscli/rules.hpp>

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
    ( "symbolic,s",                                            "Use symbolic variant (works on RCBDDs)" )
    ( "esop_minimizer", value_with_default( &esop_minimizer ), "ESOP minizer (0: built-in, 1: exorcism); only with symbolic approach" )
    ( "new,n",                                                 "Add a new entry to the store; if not set, the current entry is overriden" )
    ;

  options_description tt_options( "Options for the truth table variant" );
  tt_options.add_options()
    ( "ordering",       value( &ordering ),                    "Complete variable ordering (space separated, only for truth table variant)" )
    ;

  options_description symb_options( "Options for the symbolic variant" );
  symb_options.add_options()
    ( "mode",           value_with_default( &mode ),           "Mode (0: default, 1: swap, 2: hamming)" )
    ;

  opts.add( tt_options );
  opts.add( symb_options );

  be_verbose();
}

command::rules_t dbs_command::validity_rules() const
{
  return {
    { [&]() { return !this->is_set( "symbolic" ) || env->store<rcbdd>().current_index() >= 0u; }, "symbolic method require RCBDD in store" },
    { [&]() { return this->is_set( "symbolic" ) || env->store<binary_truth_table>().current_index() >= 0u; }, "no truth table in store" }
  };
}

bool dbs_command::execute()
{
  auto& circuits = env->store<circuit>();
  auto& rcbdds   = env->store<rcbdd>();
  auto& specs    = env->store<binary_truth_table>();

  auto settings = make_settings();
  auto statistics = std::make_shared<properties>();

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
