/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2016  EPFL
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
