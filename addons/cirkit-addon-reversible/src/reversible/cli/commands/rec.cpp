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

#include "rec.hpp"

#include <iostream>

#include <core/utils/program_options.hpp>
#include <reversible/cli/stores.hpp>
#include <reversible/verification/xorsat_equivalence_check.hpp>

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

rec_command::rec_command( const environment::ptr& env )
  : cirkit_command( env, "Equivalence checking for reversible circuits", "[L.G. Amaru, P.-E. Gaillardon, R. Wille, and G. De Micheli, DATE 2016]" )
{
  opts.add_options()
    ( "id1",            value_with_default( &id1 ), "ID of first circuit" )
    ( "id2",            value_with_default( &id2 ), "ID of second circuit" )
    ( "name_mapping,n",                             "map circuits by name instead by index" )
    ;
  be_verbose();
}

bool rec_command::execute()
{
  const auto& circuits = env->store<circuit>();

  auto settings = make_settings();
  settings->set( "name_mapping", is_set( "name_mapping" ) );
  const auto result = xorsat_equivalence_check( circuits[id1], circuits[id2], settings, statistics );

  if ( result )
  {
    std::cout << "[i] circuits are equivalent" << std::endl;
  }
  else
  {
    std::cout << "[i] circuits are not equivalent" << std::endl;
  }

  return true;
}

command::log_opt_t rec_command::log() const
{
  return boost::none;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
