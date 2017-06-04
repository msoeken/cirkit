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

boost::dynamic_bitset<>& inc_pos( boost::dynamic_bitset<>& bitset, const boost::dynamic_bitset<>& mask )
{
  auto pos = mask.find_first();
  while ( pos != boost::dynamic_bitset<>::npos )
  {
    bitset.flip( pos );
    if ( bitset.test( pos ) ) break;
    pos = mask.find_next( pos );
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

std::string bitset_join( const boost::dynamic_bitset<>& b, const std::string& sep )
{
  std::stringstream s;
  auto first = true;
  for ( auto i = 0u; i < b.size(); ++i )
  {
    if ( !first )
    {
      s << sep;
    }
    else
    {
      first = false;
    }

    s << b[i];
  }
  return s.str();
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
