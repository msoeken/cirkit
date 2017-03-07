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

#include "read_unateness.hpp"

#include <fstream>

#include <boost/range/algorithm.hpp>

#include <classical/utils/aig_utils.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

void erase_eol( std::string& s )
{
  s.erase( boost::remove( s, '\n' ), s.end() );
  s.erase( boost::remove( s, '\r' ), s.end() );
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

void read_unateness( aig_graph& aig, const std::string& filename )
{
  auto& info = aig_info( aig );

  const auto n = info.inputs.size();
  const auto m = info.outputs.size();

  boost::dynamic_bitset<> u( ( m * n ) << 1u );

  std::string line;
  std::ifstream of( filename.c_str(), std::ifstream::in );

  auto pos = 0u;
  while ( std::getline( of, line ) )
  {
    erase_eol( line );
    assert( line.size() == n );

    for ( auto c : line )
    {
      switch ( c )
      {
      case ' ':
        u[pos++] = 1; u[pos++] = 1;
        break;
      case 'p':
        u[pos++] = 0; u[pos++] = 1;
        break;
      case 'n':
        u[pos++] = 1; u[pos++] = 0;
        break;
      case '.':
        u[pos++] = 0; u[pos++] = 0;
        break;
      }
    }
  }

  assert( pos == u.size() );

  info.unateness = u;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
