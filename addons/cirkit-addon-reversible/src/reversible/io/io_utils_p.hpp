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

/**
 * @file io_utils_p.hpp
 *
 * @brief I/O helper functions
 *
 * @author Mathias Soeken
 * @since  1.0
 */

/** @cond */
#ifndef IO_UTILS_P_HPP
#define IO_UTILS_P_HPP

#include <boost/assign/std/vector.hpp>

using namespace boost::assign;

namespace cirkit
{

  template<typename Iterator>
  Iterator in_cube_to_values( binary_truth_table::in_const_iterator first, binary_truth_table::in_const_iterator last, Iterator result )
  {
    // Example
    // 0--11
    // base = 00011 = 3
    // dc_positions = 1,2
    // numbers = 3, 7, 11, 15

    unsigned base = 0;
    std::vector<unsigned> dc_positions;
    unsigned pos = 0;

    while ( first != last )
    {
      if ( *first ) // if not DC
      {
        base |= ( **first << ( last - first - 1 ) );
      }
      else
      {
        dc_positions += pos;
      }

      ++pos;
      ++first;
    }

    *result++ = base;

    for ( unsigned i = 1; i < ( 1u << dc_positions.size() ); ++i )
    {
      unsigned copy = base;
      for ( unsigned j = 0; j < dc_positions.size(); ++j )
      {
        unsigned local_bit = i & ( 1u << ( dc_positions.size() - j - 1 ) ) ? 1 : 0;
        unsigned global_bit_pos = pos - dc_positions.at( j ) - 1;
        copy |= ( local_bit << global_bit_pos );
      }
      *result++ = copy;
    }

    return result;
  }

}

#endif /* IO_UTILS_P_HPP */
/** @endcond */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
