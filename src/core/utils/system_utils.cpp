/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
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

#include "system_utils.hpp"

#include <cstdlib>
#include <fstream>
#include <iostream>

#include <boost/algorithm/string/trim.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/format.hpp>

using namespace boost::assign;

namespace cirkit
{

std::vector<std::string> execute_and_return( const std::string& cmd )
{
  std::vector<std::string> result;

  auto sresult = system( boost::str( boost::format( "( %s ) > /tmp/er.log" ) % cmd ).c_str() );
  std::ifstream is( "/tmp/er.log", std::ifstream::in );
  std::string line;
  while ( getline( is, line ) )
  {
    boost::trim( line );
    result += line;
  }
  is.close();

  return result;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
