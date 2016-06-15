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

#include "revsimp.hpp"

#include <alice/rules.hpp>
#include <reversible/circuit.hpp>
#include <reversible/cli/stores.hpp>
#include <reversible/optimization/simplify.hpp>

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

revsimp_command::revsimp_command( const environment::ptr& env )
  : cirkit_command( env, "Reversible circuit simplification" )
{
  be_verbose();
  add_new_option();
}

command::rules_t revsimp_command::validity_rules() const
{
  return {has_store_element<circuit>( env )};
}

bool revsimp_command::execute()
{
  auto& circuits = env->store<circuit>();

  auto settings = make_settings();
  circuit circ;
  simplify( circ, circuits.current(), settings, statistics );

  extend_if_new( circuits );
  circuits.current() = circ;

  print_runtime();

  return true;
}

command::log_opt_t revsimp_command::log() const
{
  return log_opt_t({{"runtime", statistics->get<double>( "runtime" )}});
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
