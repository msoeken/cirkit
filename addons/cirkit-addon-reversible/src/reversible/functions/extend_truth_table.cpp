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

#include "extend_truth_table.hpp"

#include <boost/range/iterator_range.hpp>

#include "../io/io_utils_p.hpp"

namespace cirkit
{

  template<typename Iterator>
  Iterator in_cube_to_full_cubes( binary_truth_table::in_const_iterator first, binary_truth_table::in_const_iterator last, Iterator result )
  {
    // Example
    // 0--11
    // base = 00011 = 3
    // dc_positions = 1,2
    // numbers = 3, 7, 11, 15

    binary_truth_table::cube_type cube( first, last );

    std::vector<unsigned> dc_positions;
    unsigned pos = 0;

    while ( first != last )
    {
      if ( !*first ) // if DC
      {
        dc_positions += pos;
      }

      ++pos;
      ++first;
    }

    for ( unsigned i = 0; i < ( 1u << dc_positions.size() ); ++i )
    {
      for ( unsigned j = 0; j < dc_positions.size(); ++j )
      {
        unsigned local_bit = i & ( 1u << ( dc_positions.size() - j - 1 ) ) ? 1 : 0;
        cube.at( dc_positions.at( j ) ) = local_bit;
      }
      *result++ = cube;
    }

    return result;
  }

  void extend_truth_table( binary_truth_table& spec )
  {

    typedef std::map<std::vector<binary_truth_table::cube_type>, binary_truth_table::cube_type> cube_map;
    cube_map new_cubes;

    for ( binary_truth_table::const_iterator it = spec.begin(); it != spec.end(); ++it )
    {
      // fully specified outputs
      assert( std::find( it->second.first, it->second.second, constant() ) == it->second.second );

      std::vector<binary_truth_table::cube_type> in_cube_values;
      in_cube_to_full_cubes( it->first.first, it->first.second, std::back_inserter( in_cube_values ) );

      binary_truth_table::cube_type output( it->second.first, it->second.second );

      new_cubes.insert( std::make_pair( in_cube_values, output ) );
    }

    spec.clear();

    for ( cube_map::const_iterator it = new_cubes.begin(); it != new_cubes.end(); ++it )
    {
      for ( std::vector<binary_truth_table::cube_type>::const_iterator itCube = it->first.begin(); itCube != it->first.end(); ++itCube )
      {
        spec.add_entry( *itCube, it->second );
      }
    }

    // fill the outputs (truth table is ordered)
    binary_truth_table::cube_type out_cube( spec.num_outputs(), false );
    unsigned current_pos = 0;
    for ( binary_truth_table::const_iterator it = spec.begin(); ; ++it )
    {
      unsigned pos = 0;
      unsigned i = spec.num_inputs();

      // vector to number
      if ( it == spec.end() ) // after the end
      {
        pos = 1u << spec.num_inputs();
      }
      else
      {
        for ( const auto& in_bit : boost::make_iterator_range( it->first ) )
        {
          pos |= *in_bit << --i;
        }
      }

      for ( i = current_pos; i < pos; ++i )
      {
        spec.add_entry( number_to_truth_table_cube( i, spec.num_inputs() ), out_cube );
      }

      current_pos = pos;

      // there is no <= comparison possible
      if ( it == spec.end() )
      {
        break;
      }
    }

  }

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
