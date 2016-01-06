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

#include "rcbdd.hpp"

#include <reversible/circuit.hpp>
#include <reversible/rcbdd.hpp>
#include <reversible/cli/stores.hpp>

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

rcbdd_command::rcbdd_command( const environment::ptr& env )
  : command( env, "RCBDD functions" )
{
  opts.add_options()
    ( "circuit,c", "Read from current circuit" )
    ( "new,n",     "Add a new entry to the store; if not set, the current entry is overriden" )
    ;
}


command::rules_t rcbdd_command::validity_rules() const
{
  return { { [&]() { return !opts.is_set( "circuit" ) || env->store<circuit>().current_index() >= 0; }, "no circuit in store" } };
}

bool rcbdd_command::execute()
{
  if ( opts.is_set( "circuit" ) )
  {
    auto& rcbdds   = env->store<rcbdd>();
    auto& circuits = env->store<circuit>();

    if ( rcbdds.empty() || opts.is_set( "new" ) )
    {
      rcbdds.extend();
    }

    const auto& circ = circuits.current();

    rcbdd cf;
    cf.initialize_manager();
    cf.create_variables( circ.lines() );
    cf.set_chi( cf.create_from_circuit( circ ) );

    rcbdds.current() = cf;
  }
  return true;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
