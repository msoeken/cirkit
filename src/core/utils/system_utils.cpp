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

#include "system_utils.hpp"

#include <cstdlib>
#include <fstream>
#include <iostream>

#include <boost/algorithm/string/trim.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/format.hpp>

#include <sys/wait.h>
#include <unistd.h>

using namespace boost::assign;

namespace cirkit
{

result_t execute_and_return( const std::string& cmd, const std::string& pattern )
{
  std::vector<std::string> result;

  const std::string filename = ( boost::format( "/tmp/er-%s.log" ) % getpid() ).str();
  auto sresult = system( boost::str( boost::format( pattern ) % cmd % filename ).c_str() );
  std::ifstream is( filename.c_str(), std::ifstream::in );
  std::string line;
  while ( getline( is, line ) )
  {
    boost::trim( line );
    result += line;
  }
  is.close();

  return { WEXITSTATUS( sresult ), result };
}

result_t execute_and_omit( const std::string& cmd )
{
  auto sresult = system( boost::str( boost::format( "( %s ) > /dev/null" ) % cmd ).c_str() );
  return { WEXITSTATUS( sresult ), std::vector<std::string>() };
}

result_t execute_and_return( const std::string& cmd )
{
  return execute_and_return( cmd, "( %s ) > %s" );
}

result_t execute_and_return_tee( const std::string& cmd )
{
  return execute_and_return( cmd, "( %s ) | tee %s" );
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
