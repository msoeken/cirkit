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

#include "bdd.hpp"

#include <vector>

#include <lscli/rules.hpp>

#include <core/cli/stores.hpp>

using namespace boost::program_options;

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

bdd_command::bdd_command( const environment::ptr& env )
  : cirkit_command( env, "BDD manipulation" )
{
  opts.add_options()
    ( "characteristic,c", value( &characteristic ), "Compute characteristic function (x: inputs first, y: outputs first)" )
    ( "new,n",                                      "Add a new entry to the store; if not set, the current entry is overriden" )
    ;
}

command::rules_t bdd_command::validity_rules() const
{
  return { has_store_element<bdd_function_t>( env ) };
}

bool bdd_command::execute()
{
  auto& bdds = env->store<bdd_function_t>();

  if ( is_set( "characteristic" ) )
  {
    auto bdd = bdds.current();

    if ( is_set( "new" ) )
    {
      bdds.extend();
    }

    bdds.current() = compute_characteristic( bdd, characteristic == "x" );
  }

  return true;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
