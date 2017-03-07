/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2017  EPFL
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
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
