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

#include "cirkit_command.hpp"

#include <iostream>
#include <memory>

#include <boost/format.hpp>

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

cirkit_command::cirkit_command( const environment::ptr& env, const std::string& caption, const std::string& publications )
  : command( env, caption, publications )
{
}

bool cirkit_command::run( const std::vector<std::string>& args )
{
  statistics.reset( new properties() );
  return command::run( args );
}

properties::ptr cirkit_command::make_settings() const
{
  auto settings = std::make_shared<properties>();
  if ( opts.find_nothrow( "verbose", false ) )
  {
    settings->set( "verbose", is_verbose() );
  }
  return settings;
}

void cirkit_command::print_runtime( const std::string& key, const std::string& label ) const
{
  if ( label.empty() )
  {
    std::cout << boost::format( "[i] run-time: %.2f secs" ) % statistics->get<double>( key ) << std::endl;
  }
  else
  {
    std::cout << boost::format( "[i] run-time (%s): %.2f secs" ) % label % statistics->get<double>( key ) << std::endl;
  }
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
