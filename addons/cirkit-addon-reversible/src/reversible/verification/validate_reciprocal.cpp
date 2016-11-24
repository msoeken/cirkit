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
