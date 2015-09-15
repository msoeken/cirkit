/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
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

/**
 * @file bitset_utils.hpp
 *
 * @brief Some helper functions for bitsets
 *
 * @author Mathias Soeken
 * @author Arun Chandrasekharan
 * @since  2.0
 */

#ifndef BITSET_UTILS_HPP
#define BITSET_UTILS_HPP

#include <iostream>
#include <random>
#include <vector>

#include <boost/dynamic_bitset.hpp>

namespace cirkit
{

boost::dynamic_bitset<>& inc( boost::dynamic_bitset<>& bitset );

std::vector<boost::dynamic_bitset<>> transpose( const std::vector<boost::dynamic_bitset<>>& vs );

template<class URNG>
boost::dynamic_bitset<> random_bitset( unsigned n, URNG& g )
{
  std::uniform_int_distribution<unsigned long> dist( 0ul, std::numeric_limits<unsigned long>::max() );

  boost::dynamic_bitset<> b( n );
  unsigned pos = 0u;
  while ( pos < n )
  {
    boost::dynamic_bitset<> c( sizeof( unsigned long ) * 8u, dist( g ) );
    auto to = std::min( c.size(), b.size() - pos );
    for ( unsigned i = 0u; i < to; ++i )
    {
      b[pos + i] = c[i];
    }
    pos += c.size();
  }
  return b;
}

boost::dynamic_bitset<> random_bitset( unsigned n );

std::ostream& print_as_set( std::ostream& os, const boost::dynamic_bitset<>& b );

boost::dynamic_bitset<> onehot_bitset( unsigned n, unsigned pos );

template<typename Fn>
void foreach_bit( const boost::dynamic_bitset<>& b, const Fn&& func )
{
  auto pos = b.find_first();
  while ( pos != boost::dynamic_bitset<>::npos )
  {
    func( pos );
    pos = b.find_next( pos );
  }
}

std::vector<unsigned> get_index_vector( const boost::dynamic_bitset<> b );

template<class IntType>
IntType to_multiprecision( const boost::dynamic_bitset<>& bs )
{
  IntType sum = 0;
  const IntType one = 1;

  foreach_bit( bs, [&]( unsigned pos ) { sum |= (one << pos); } );

  return sum;
}

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
