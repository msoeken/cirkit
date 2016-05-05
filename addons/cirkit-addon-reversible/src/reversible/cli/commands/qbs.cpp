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

#include "qbs.hpp"

#include <boost/format.hpp>

#include <core/cli/rules.hpp>
#include <core/utils/program_options.hpp>
#include <classical/optimization/esop_minimization.hpp>
#include <classical/optimization/exorcism_minimization.hpp>
#include <reversible/circuit.hpp>
#include <reversible/rcbdd.hpp>
#include <reversible/cli/stores.hpp>
#include <reversible/synthesis/qmdd_synthesis.hpp>

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

qbs_command::qbs_command( const environment::ptr& env )
  : cirkit_command( env, "QMDD based synthesis" )
{
  be_verbose();
  opts.add_options()
    ( "esop_minimizer", value_with_default( &esop_minimizer ), "ESOP minizer (0: built-in, 1: exorcism); only with symbolic approach" )
    ( "new,n",                                                 "Creates new entry in store for circuit" )
    ;
}

command::rules_t qbs_command::validity_rules() const
{
  return { has_store_element<rcbdd>( env ) };
}

bool qbs_command::execute()
{
  const auto& rcbdds = env->store<rcbdd>();
  auto& circuits = env->store<circuit>();

  if ( circuits.empty() || is_set( "new" ) )
  {
    circuits.extend();
  }

  const auto settings = make_settings();

  auto esopmin_settings = std::make_shared<properties>();
  esopmin_settings->set( "verbose", is_verbose() );
  settings->set( "esopmin", esop_minimizer ? dd_based_exorcism_minimization_func( esopmin_settings ) : dd_based_esop_minimization_func( esopmin_settings ) );

  qmdd_synthesis( circuits.current(), rcbdds.current(), settings, statistics );

  std::cout << boost::format( "[i] run-time: %.2f secs" ) % statistics->get<double>( "runtime" ) << std::endl;

  return true;
}

command::log_opt_t qbs_command::log() const
{
  return log_opt_t({
      {"runtime", statistics->get<double>( "runtime" )}
    });
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
