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

#include "circuit_to_truth_table.hpp"

#include <boost/format.hpp>

#include <core/properties.hpp>
#include <core/utils/bitset_utils.hpp>

namespace cirkit
{

  void bitset_to_vector( binary_truth_table::cube_type& vec, const boost::dynamic_bitset<> number )
  {
    vec.clear();
    for ( unsigned i = 0u; i < number.size(); ++i )
    {
      vec.push_back( number.test( i ) );
    }
  }

  bool circuit_to_truth_table( const circuit& circ, binary_truth_table& spec, const functor<bool(boost::dynamic_bitset<>&, const circuit&, const boost::dynamic_bitset<>&)>& simulation )
  {
    // number of patterns to check depends on partial or non-partial simulation
    boost::dynamic_bitset<>::size_type n = ( simulation.settings() && simulation.settings()->get<bool>( "partial", false ) ) ? std::count( circ.constants().begin(), circ.constants().end(), constant() ) : circ.lines();
    boost::dynamic_bitset<> input( n, 0u );

    do
    {
      boost::dynamic_bitset<> output;

      if ( simulation( output, circ, input ) )
      {
        binary_truth_table::cube_type in_cube, out_cube;

        bitset_to_vector( in_cube, input );
        bitset_to_vector( out_cube, output );

        spec.add_entry( in_cube, out_cube );
      }
      else
      {
        return false;
      }
    } while ( !inc( input ).none() );

    // metadata
    spec.set_inputs( circ.inputs() );
    spec.set_outputs( circ.outputs() );
    spec.set_constants( circ.constants() );
    spec.set_garbage( circ.garbage() );

    return true;
  }

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
