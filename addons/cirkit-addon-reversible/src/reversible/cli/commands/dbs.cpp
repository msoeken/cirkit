/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
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

#include <core/cli/rules.hpp>
#include <core/cli/store.hpp>
#include <core/utils/program_options.hpp>
#include <classical/optimization/esop_minimization.hpp>
#include <classical/optimization/exorcism_minimization.hpp>
#include <reversible/circuit.hpp>
#include <reversible/rcbdd.hpp>
#include <reversible/truth_table.hpp>
#include <reversible/cli/stores.hpp>
#include <reversible/synthesis/rcbdd_synthesis.hpp>
#include <reversible/synthesis/young_subgroup_synthesis.hpp>

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

dbs_command::dbs_command( const environment::ptr& env )
  : command( env, "Decomposition based synthesis" )
{
  opts.add_options()
    ( "symbolic,s",                                            "Use symbolic variant (works on RCBDDs)" )
    ( "esop_minimizer", value_with_default( &esop_minimizer ), "ESOP minizer (0: built-in, 1: exorcism); only with symbolic approach" )
    ( "new,n",                                                 "Add a new entry to the store; if not set, the current entry is overriden" )
    ;
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

  if ( circuits.empty() || opts.is_set( "new" ) )
  {
    circuits.extend();
  }

  circuit circ;

  auto esopmin_settings = std::make_shared<properties>();
  esopmin_settings->set( "verbose", is_verbose() );
  settings->set( "esopmin", esop_minimizer ? dd_based_exorcism_minimization_func( esopmin_settings ) : dd_based_esop_minimization_func( esopmin_settings ) );

  if ( opts.is_set( "symbolic" ) )
  {
    rcbdd_synthesis( circ, rcbdds.current(), settings, statistics );
  }
  else
  {
    young_subgroup_synthesis( circ, specs.current(), settings, statistics );
  }

  circuits.current() = circ;

  std::cout << boost::format( "[i] run-time: %.2f secs" ) % statistics->get<double>( "runtime" ) << std::endl;

  return true;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
