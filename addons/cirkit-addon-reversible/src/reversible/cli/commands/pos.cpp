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

#include "pos.hpp"

#include <alice/rules.hpp>

#include <reversible/cli/stores.hpp>
#include <reversible/functions/negative_controls_to_positive.hpp>

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

pos_command::pos_command( const environment::ptr& env )
  : cirkit_command( env, "Replace negative controls with positive ones" )
{
  add_new_option();
}

command::rules_t pos_command::validity_rules() const
{
  return {has_store_element<circuit>( env )};
}

bool pos_command::execute()
{
  auto& circuits = env->store<circuit>();

  circuit circ_new;
  negative_controls_to_positive( circuits.current(), circ_new );

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
