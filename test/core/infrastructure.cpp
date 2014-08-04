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
#define BOOST_TEST_MODULE infrastructure

#include <boost/test/unit_test.hpp>

#include <core/utils/benchmark_table.hpp>

using namespace cirkit;

BOOST_AUTO_TEST_CASE(simple)
{
  benchmark_table<std::string, unsigned, double> table( { "Name", "Gates", "Time" } );

  table.add( std::string( "hwb4" ),  10u, 4.50 );
  table.add( std::string( "sqrt2" ), 18u, 9.20 );
  table.add( std::string( "d2" ),     2u, 1.01 );

  table.print();
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
