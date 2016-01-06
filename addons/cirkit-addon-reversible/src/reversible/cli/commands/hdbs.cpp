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

#include "hdbs.hpp"

#include <boost/format.hpp>

#include <core/utils/bdd_utils.hpp>
#include <core/cli/rules.hpp>
#include <core/cli/store.hpp>
#include <core/cli/stores.hpp>
#include <core/utils/timer.hpp>
#include <reversible/circuit.hpp>
#include <reversible/cli/stores.hpp>
#include <reversible/synthesis/dd_synthesis_p.hpp>

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

hdbs_command::hdbs_command( const environment::ptr& env )
  : command( env, "Hierarhical DD-based synthesis." )
{
  opts.add_options()
    ( "new,n", "Add new circuit to store" )
    ;
}

command::rules_t hdbs_command::valididity_rules() const
{
  return { has_store_element<bdd_function_t>( env ) };
}

bool hdbs_command::execute()
{
  auto& bdd = env->store<bdd_function_t>().current();

  circuit circ;

  {
    properties_timer t( statistics );
    using namespace internal;
    dd graph;
    dd_from_bdd_settings settings;
    settings.complemented_edges = true;
    settings.reordering = 4;
    dd_from_bdd( graph, bdd, settings );
    dd_synthesis( circ, graph );
  }

  std::cout << boost::format( "[i] run-time: %.2f secs" ) % statistics->get<double>( "runtime" ) << std::endl;

  auto& circuits = env->store<circuit>();
  if ( circuits.empty() || opts.is_set( "new" ) )
  {
    circuits.extend();
  }
  circuits.current() = circ;

  return true;
}

command::log_opt_t hdbs_command::log() const
{
  return log_opt_t( {{"runtime", statistics->get<double>( "runtime" )}} );
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
