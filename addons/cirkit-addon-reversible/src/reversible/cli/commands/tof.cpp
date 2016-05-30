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

#include "tof.hpp"

#include <alice/rules.hpp>

#include <reversible/cli/stores.hpp>
#include <reversible/functions/fredkin_gates_to_toffoli.hpp>

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

tof_command::tof_command( const environment::ptr& env )
  : cirkit_command( env, "Rewrite Fredkin gates to Toffoli gates" )
{
  add_new_option();
}

command::rules_t tof_command::validity_rules() const
{
  return {has_store_element<circuit>( env )};
}

bool tof_command::execute()
{
  auto& circuits = env->store<circuit>();

  circuit circ_new;
  fredkin_gates_to_toffoli( circuits.current(), circ_new );

  extend_if_new( circuits );
  circuits.current() = circ_new;

  return true;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
