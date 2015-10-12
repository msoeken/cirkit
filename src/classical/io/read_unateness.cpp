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
