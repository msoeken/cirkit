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

#include "validate_reciprocal.hpp"

#include <boost/dynamic_bitset.hpp>
#include <boost/multiprecision/cpp_int.hpp>

#include <core/utils/bitset_utils.hpp>
#include <core/utils/timer.hpp>
#include <reversible/simulation/partial_simulation.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

void validate_reciprocal( const circuit& circ, unsigned bitwidth, const properties::ptr& settings, const properties::ptr& statistics )
{
  /* settings */
  const auto rounds = get( settings, "rounds", 100u );

  /* statistics */
  properties_timer t( statistics );

  for ( auto i = 0u; i < rounds; ++i )
  {
    const auto assignment = random_bitset( bitwidth );
    boost::dynamic_bitset<> output;

    partial_simulation( output, circ, assignment );

    assert( output.size() == bitwidth );

    const auto x = to_multiprecision<boost::multiprecision::uint256_t>( assignment );
    const auto y = to_multiprecision<boost::multiprecision::uint256_t>( output );

    boost::multiprecision::uint256_t N = 1;
    N = N << bitwidth;

    if ( x != 0 && N / x != y )
    {
      std::cout << "[w] inconsistency detected:" << std::endl;
      std::cout << "[w] x = " << x << ", y = " << y << ", x * y = " << ( x * y ) << std::endl;
      std::cout << "[w] N / x = " << ( N / x ) << std::endl;
    }
    else
    {
      std::cout << "[i] x = " << x << " y = " << y << std::endl;
    }
  }
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
