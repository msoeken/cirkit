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

#include "bitset_utils.hpp"

#include <chrono>

#include <boost/assign/std/vector.hpp>

using namespace boost::assign;

namespace cirkit
{

boost::dynamic_bitset<>& inc( boost::dynamic_bitset<>& bitset )
{
  for ( boost::dynamic_bitset<>::size_type i = 0; i < bitset.size(); ++i )
  {
    bitset.flip( i );
    if ( bitset.test( i ) ) break;
  }
  return bitset;
}

std::vector<boost::dynamic_bitset<>> transpose( const std::vector<boost::dynamic_bitset<>>& vs )
{
  std::vector<boost::dynamic_bitset<>> ts( vs.front().size(), boost::dynamic_bitset<>( vs.size() ) );

  for ( unsigned i = 0u; i < vs.size(); ++i )
  {
    for ( unsigned j = 0u; j < vs[i].size(); ++j )
    {
      ts[j][i] = vs[i][j];
    }
  }

  return ts;
}

boost::dynamic_bitset<> random_bitset( unsigned n )
{
  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::default_random_engine generator( seed );
  return random_bitset( n, generator );
}

boost::dynamic_bitset<> sub_bitset( const boost::dynamic_bitset<>& b, unsigned from, unsigned to )
{
  boost::dynamic_bitset<> bs( to - from );
  for ( auto i = from; i < to; ++i )
  {
    bs[i - from] = b[i];
  }
  return bs;
}

std::ostream& print_as_set( std::ostream& os, const boost::dynamic_bitset<>& b )
{
  auto pos = b.find_first();

  os << "{ ";

  while ( pos != boost::dynamic_bitset<>::npos )
  {
    os << pos << " ";
    pos = b.find_next( pos );
  }

  return os << "}";
}

boost::dynamic_bitset<> onehot_bitset( unsigned n, unsigned pos )
{
  boost::dynamic_bitset<> b( n );
  b.set( pos );
  return b;
}

std::vector<unsigned> get_index_vector( const boost::dynamic_bitset<> b )
{
  std::vector<unsigned> v;

  auto pos = b.find_first();
  while ( pos != boost::dynamic_bitset<>::npos )
  {
    v += pos;
    pos = b.find_next( pos );
  }

  return v;
}

std::string to_string( const boost::dynamic_bitset<>& b )
{
  std::string s;
  to_string( b, s );
  return s;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
