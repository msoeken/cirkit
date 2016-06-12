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

#include "memristor.hpp"

#include <iostream>

#include <classical/utils/memristor_costs.hpp>

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

memristor_command::memristor_command( const environment::ptr& env )
  : mig_base_command( env, "Memristor operations" )
{
  opts.add_options()
    ( "costs,c", "Show memristor costs" )
    ;
}

bool memristor_command::execute()
{
  if ( is_set( "costs" ) )
  {
    std::tie( memristors, operations ) = memristor_costs( mig() );
    std::cout << "[i] #memristors: " << memristors << " #operations: " << operations << std::endl;
  }

  return true;
}

command::log_opt_t memristor_command::log() const
{
  return log_opt_t( {{"memristors", static_cast<int>( memristors )},
                     {"operations", static_cast<int>( operations )}} );
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
