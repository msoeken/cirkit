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


#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE timers

#include <boost/test/output_test_stream.hpp>
#define timer timer_class
#include <boost/test/included/unit_test.hpp>
#undef timer

#include <core/utils/timer.hpp>

using namespace cirkit;

BOOST_AUTO_TEST_CASE(simple)
{
  using boost::test_tools::output_test_stream;

  output_test_stream o1, o2;
  {
    print_timer pt( "This message should be visible after %w", true, o1 );
  }
  BOOST_CHECK( !o1.is_empty() );

  {
    print_timer pt( "This message should not be visible after %w", false, o2 );
  }
  BOOST_CHECK( o2.is_empty() );

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
