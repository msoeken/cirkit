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

#include "write_qpic.hpp"

#include <fstream>
#include <vector>

#include <boost/algorithm/string/join.hpp>
#include <boost/format.hpp>

#include <reversible/target_tags.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

void write_qpic( const circuit& circ, std::ostream& os )
{
  for ( auto i = 0u; i < circ.lines(); ++i )
  {
    os << boost::format( "l%d W %s %s" ) % i % circ.inputs()[i] % circ.outputs()[i] << std::endl;
  }

  for ( const auto& g : circ )
  {
    assert( is_toffoli( g ) );

    std::vector<std::string> items;

    for ( const auto& c : g.controls() )
    {
      items.push_back( boost::str( boost::format( "%sl%d" ) % ( c.polarity() ? "" : "-" ) % c.line() ) );
    }

    for ( const auto& t : g.targets() )
    {
      items.push_back( boost::str( boost::format( "+l%d" ) % t ) );
    }

    os << boost::join( items, " " ) << std::endl;
  }
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

void write_qpic( const circuit& circ, const std::string& filename )
{
  std::ofstream os( filename.c_str(), std::ofstream::out );
  write_qpic( circ, os );
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
