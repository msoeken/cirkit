/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2017  EPFL
 * Copyright (C) 2015  The Regents of the University of California
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
