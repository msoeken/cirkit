/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015  The Regents of the University of California
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
#define BOOST_TEST_MODULE index_backtracking_set

#include <boost/test/included/unit_test.hpp>

#include <core/utils/index.hpp>

using namespace cirkit;

struct test_index_tag;
using test_index = base_index<test_index_tag>;

BOOST_AUTO_TEST_CASE(simple)
{
  using backtracking_set = index_backtracking_set<test_index>;

  backtracking_set x;

  for(unsigned i=0 ; i<10; i++)
  {
    BOOST_CHECK( ! x.insert(test_index::from_index(i)) );
  }

  BOOST_CHECK( x.size() == 10);

  x.save_state();

  BOOST_CHECK( x.remove(test_index::from_index(5) ) );

  BOOST_CHECK( x.size() == 9);

  x.save_state();

  BOOST_CHECK( x.remove(test_index::from_index(4) ) );

  BOOST_CHECK( x.size() == 8);

  BOOST_CHECK( x.has(test_index::from_index(3) ) );
  BOOST_CHECK( ! x.has(test_index::from_index(4) ) );
  BOOST_CHECK( ! x.has(test_index::from_index(5) ) );

  backtracking_set x8;

  for(unsigned i=0 ; i<10; i++)
  {
    if( i==4 || i==5 )
    {
      continue;
    }

    BOOST_CHECK( ! x8.insert(test_index::from_index(i)) );
  }

  BOOST_CHECK( x==x8 );

  x.restore_state();

  BOOST_CHECK( ! (x==x8) );

  x8.insert( test_index::from_index(4) );

  BOOST_CHECK( x==x8 );

  BOOST_CHECK( x.has(test_index::from_index(3) ) );
  BOOST_CHECK( x.has(test_index::from_index(4) ) );
  BOOST_CHECK( ! x.has(test_index::from_index(5) ) );

  BOOST_CHECK( x.size() == 9);

  x.restore_state();

  BOOST_CHECK( x.size() == 10);

  BOOST_CHECK( x.has(test_index::from_index(3) ) );
  BOOST_CHECK( x.has(test_index::from_index(4) ) );
  BOOST_CHECK( x.has(test_index::from_index(5) ) );
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
