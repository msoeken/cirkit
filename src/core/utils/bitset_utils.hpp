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

#include <algorithm>
#include <iostream>
#include <random>
#include <sstream>
#include <vector>

#include <boost/dynamic_bitset.hpp>

namespace cirkit
{

boost::dynamic_bitset<>& inc( boost::dynamic_bitset<>& bitset );
boost::dynamic_bitset<>& inc_pos( boost::dynamic_bitset<>& bitset, const boost::dynamic_bitset<>& mask );

template<typename Fn>
void foreach_bitset( unsigned num_bits, const Fn&& func )
{
  boost::dynamic_bitset<> bs( num_bits );

  do
  {
    func( bs );
    inc( bs );
  } while ( bs.any() );
}

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
boost::dynamic_bitset<> sub_bitset( const boost::dynamic_bitset<>& b, unsigned from, unsigned to );

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
IntType to_multiprecision( const boost::dynamic_bitset<>& bs, bool reverse = false )
{
  IntType sum = 0;
  const IntType one = 1;

  if ( reverse )
  {
    foreach_bit( bs, [&]( unsigned pos ) { sum |= (one << (bs.size() - pos - 1)); } );
  }
  else
  {
    foreach_bit( bs, [&]( unsigned pos ) { sum |= (one << pos); } );
  }

  return sum;
}

std::string to_string( const boost::dynamic_bitset<>& b );
std::string bitset_join( const boost::dynamic_bitset<>& b, const std::string& sep = " " );
template<class ContainerType>
std::string bitset_indexed_join( const boost::dynamic_bitset<>& b, const ContainerType& indexes, const std::string& sep = " ", int offset = 0 )
{
  std::stringstream s;
  auto first = true;
  for ( auto i : indexes )
  {
    if ( !first )
    {
      s << sep;
    }
    else
    {
      first = false;
    }

    s << b[i + offset];
  }
  return s.str();
}


}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
