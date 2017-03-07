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

#include "truth_table.hpp"

#include <boost/range/iterator_range.hpp>

namespace cirkit
{

  std::ostream& operator<<( std::ostream& os, const binary_truth_table& spec )
  {
    for ( binary_truth_table::const_iterator it = spec.begin(); it != spec.end(); ++it )
    {
      // iterate through input cube (bit by bit)
      for ( const auto& in_bit : boost::make_iterator_range( it->first ) )
      {
        os << ( in_bit ? ( *in_bit ? "1" : "0" ) : "-" );
      }

      os << " ";

      // iterate through output cube (bit by bit)
      for ( const auto& out_bit : boost::make_iterator_range( it->second ) )
      {
        os << ( out_bit ? ( *out_bit ? "1" : "0" ) : "-" );
      }

      os << std::endl;
    }

    return os;
  }

  unsigned truth_table_cube_to_number( const binary_truth_table::cube_type& cube )
  {
    unsigned ret = 0;

    for ( unsigned i = 0; i < cube.size(); ++i )
    {
      assert( cube.at( i ) );
      ret |= ( *cube.at( i ) << ( cube.size() - 1 - i ) );
    }

    return ret;
  }

  binary_truth_table::cube_type number_to_truth_table_cube( unsigned number, unsigned bw )
  {
    binary_truth_table::cube_type c;

    for ( unsigned i = 0; i < bw; ++i )
    {
      c.push_back( ( number & ( 1u << ( bw -1  - i ) ) ) ? true : false );
    }

    return c;
  }

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
