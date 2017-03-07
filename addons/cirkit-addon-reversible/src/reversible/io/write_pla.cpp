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
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
