/* RevKit (www.rekit.org)
 * Copyright (C) 2009-2014  University of Bremen
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

#include "write_pla.hpp"

#include <fstream>

#include <boost/algorithm/string/join.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/iterator_range.hpp>

namespace cirkit
{

void write_pla( const binary_truth_table& pla, const std::string& filename )
{
  std::filebuf fb;
  fb.open( filename.c_str(), std::ios::out );

  std::ostream os( &fb );

  auto bit_to_char = []( const boost::optional<bool>& b ) {
    return b ? (*b ? '1' : '0') : '-';
  };

  bool first = true;

  for ( binary_truth_table::const_iterator it = pla.begin(); it != pla.end(); ++it )
  {
    using boost::adaptors::transformed;

    if ( first )
    {
      unsigned num_inputs = std::distance( it->first.first, it->first.second );
      unsigned num_outputs = std::distance( it->second.first, it->second.second );

      os << ".i " << num_inputs << std::endl;
      os << ".o " << num_outputs << std::endl;

      if ( num_inputs == pla.inputs().size() )
      {
        os << ".ilb " << boost::join( pla.inputs(), " " ) << std::endl;
      }

      if ( num_outputs == pla.outputs().size() )
      {
        os << ".ob " << boost::join( pla.outputs(), " " ) << std::endl;
      }

      first = false;
    }

    for ( const auto& b : boost::make_iterator_range( it->first ) ) {
      os << bit_to_char( b );
    }
    //boost::for_each( boost::make_iterator_range( it->first ) | transformed( bit_to_char ), [&os]( char c ) { os << c; } );
    os << " ";
    for ( const auto& b : boost::make_iterator_range( it->second ) ) {
      os << bit_to_char( b );
    }
    //boost::for_each( boost::make_iterator_range( it->second ) | transformed( bit_to_char ), [&os]( char c ) { os << c; } );
    os << std::endl;
  }

  os << ".e" << std::endl;

  fb.close();

}

}

// Local Variables:
// c-basic-offset: 2
// End:
