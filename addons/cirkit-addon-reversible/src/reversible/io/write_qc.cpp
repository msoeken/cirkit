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

#include "write_qc.hpp"

#include <fstream>

#include <boost/algorithm/string/join.hpp>

#include <core/utils/range_utils.hpp>
#include <reversible/target_tags.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

void write_qc( const circuit& circ, std::ostream& os )
{
  const auto vars = create_name_list( "v%d", circ.lines() );

  os << ".v " << boost::join( vars, " " ) << std::endl
     << "BEGIN" << std::endl;

  for ( const auto& gate : circ )
  {
    assert( is_toffoli( gate ) );

    os << "t" << ( gate.controls().size() + 1 );
    for ( const auto& c : gate.controls() )
    {
      os << " " << vars[c.line()];
      if ( !c.polarity() )
      {
        os << "'";
      }
    }
    os << " " << vars[gate.targets().front()] << std::endl;
  }

  os << "END" << std::endl;
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

void write_qc( const circuit& circ, const std::string& filename )
{
  std::ofstream os( filename.c_str(), std::ofstream::out );
  write_qc( circ, os );
  os.close();
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
