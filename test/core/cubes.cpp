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

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE cubes

#include <boost/test/unit_test.hpp>

#include <core/cube.hpp>

using namespace cirkit;

BOOST_AUTO_TEST_CASE(simple)
{
  cube c1( "10100100" );
  cube c2( "-10--100" );

  BOOST_CHECK( c1.to_string() == "10100100" );
  BOOST_CHECK( c2.to_string() == "-10--100" );

  BOOST_CHECK( c1.length() == 8u );
  BOOST_CHECK( c2.length() == 8u );
  BOOST_CHECK( c1.dimension() == 8u );
  BOOST_CHECK( c2.dimension() == 5u );

  BOOST_CHECK( c1.match( c2 ) == 3u );
  BOOST_CHECK( c1.match_intersect( c2 ) == -1 );

  cube ds_c1( "----" );
  cube ds_c2( "11-0" );

  BOOST_CHECK( ds_c1.match_intersect( ds_c2 ) == 0 );

  auto result = ds_c1.disjoint_sharp( ds_c2 );
  BOOST_CHECK( result.size() == 3u );
  BOOST_CHECK( result[0u].to_string() == "0---" );
  BOOST_CHECK( result[1u].to_string() == "10--" );
  BOOST_CHECK( result[2u].to_string() == "11-1" );

  auto result2 = ds_c2.disjoint_sharp( ds_c1 );
  BOOST_CHECK( result2.empty() );
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
