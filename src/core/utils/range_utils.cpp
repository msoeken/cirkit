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

#include "range_utils.hpp"

#include <numeric>
#include <boost/format.hpp>

namespace cirkit
{

void mixed_radix( std::vector<unsigned>& a, const std::vector<unsigned>& m, const std::function<bool(const std::vector<unsigned>&)>&& func )
{
  while ( true )
  {
    /* step M2. [Visit.] */
    if ( func( a ) ) { return; }

    /* next iteration */
    auto j = a.size() - 1u;
    while ( a[j] == m[j] - 1u ) { a[j--] = 0u; }

    if ( !j ) { break; }

    a[j]++;
  }
}

void lexicographic_combinations( unsigned n, unsigned t, const std::function<bool(const std::vector<unsigned>&)>&& func )
{
  /* special cases */
  if ( t > n )
  {
    assert( false );
  }
  else if ( t == n )
  {
    std::vector<unsigned> v( n );
    std::iota( v.begin(), v.end(), 0u );
    func( v );
    return;
  }
  else if ( t == 0u )
  {
    func( {} );
    return;
  }

  /* regular case */

  /* step T1. [Initialize.] */
  std::vector<unsigned> c( t );
  std::iota( c.begin(), c.end(), 0u );
  c.push_back( n );
  c.push_back( 0u );
  const auto sentinel = c.begin() + t;
  auto j = t;
  unsigned x{};

  while ( true )
  {
    /* step T2. [Visit.] */
    if ( func( std::vector<unsigned>( c.begin(), sentinel ) ) ) return;

    if ( j > 0 )
    {
      x = j;
    }
    else
    {
      /* step T3. [Easy case?] */
      if ( c[0] + 1 < c[1] )
      {
        ++c[0];
        continue;
      }
      j = 2u;

      /* step T4. [Find j.] */
      while ( true )
      {
        c[j - 2] = j - 2;
        x = c[j - 1] + 1;
        if ( x != c[j] ) break;
        ++j;
      }

      /* step T5. [Done?] */
      if ( j > t ) break;
    }

    /* step T6. [Increase c_j.] */
    c[--j] = x;
  }
}

std::vector<std::string> create_name_list( const std::string& pattern, unsigned length, unsigned start )
{
  std::vector<std::string> names( length );

  for ( auto i = 0u; i < length; ++i )
  {
    names[i] = boost::str( boost::format( pattern ) % ( i + start ) );
  }

  return names;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
