/* RevKit (www.revkit.org)
 * Copyright (C) 2009-2014  University of Bremen
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

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE cubes

#include <boost/test/unit_test.hpp>

#include <core/cube.hpp>

#include <classical/optimization/compact_dsop.hpp>

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

  properties::ptr settings( new properties );
  settings->set( "verbose", true );
  compact_dsop( "/tmp/test.pla", "../test/tcad-1.pla", settings );
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
