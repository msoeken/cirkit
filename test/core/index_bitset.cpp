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

#include <boost/test/unit_test.hpp>

#include <core/utils/index.hpp>

using namespace cirkit;

struct test_index_tag;
using test_index = base_index<test_index_tag>;

namespace
{
  test_index I(unsigned i)
  {
    return test_index::from_index(i);
  }
}

BOOST_AUTO_TEST_CASE(simple)
{
  using bitset = index_bitset<test_index>;

  bitset x;

  // insert even numbers
  for(unsigned i=2 ; i<10; i+=2)
  {
    BOOST_CHECK_MESSAGE(!x.insert(I(i)), "Inserting: " << i);
  }

  // make sure that an index is in the set iff it is even and below 10
  for(unsigned i=1 ; i<20; i++)
  {
    BOOST_CHECK_MESSAGE( x.has(I(i)) == (i%2==0 && i<10), "Checking: " << i );
  }

  // remove all elements above 5
  for(unsigned i=6 ; i<10; i++)
  {
    x.remove( I(i) );
  }

  // check whether the right elements are in the set
  for(unsigned i=1 ; i<10; i++)
  {
    BOOST_CHECK_MESSAGE( x.has(I(i)) == (i%2==0 && i<5), "Checking: " << i << ", should be:" << (i%2==0 && i<5));
  }
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
