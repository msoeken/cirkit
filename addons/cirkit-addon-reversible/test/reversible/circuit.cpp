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
#define BOOST_TEST_MODULE circuit

#include <boost/assign/std/vector.hpp>
#include <boost/test/unit_test.hpp>

#include <reversible/circuit.hpp>
#include <reversible/target_tags.hpp>
#include <reversible/functions/add_gates.hpp>

BOOST_AUTO_TEST_CASE(simple)
{
  using namespace cirkit;
  using namespace boost::assign;

  circuit circ( 3u );
  BOOST_CHECK( circ.lines() == 3u );

  std::vector<unsigned> controls;
  controls += 0u,1u;
  append_toffoli( circ, controls, 2u );
  append_toffoli( circ )( 0u, 1u )( 2u );
  append_cnot( circ, 0u, 1u );
  append_not( circ, 0u );

  BOOST_CHECK( circ.num_gates() == 4u );
  unsigned i = 0u;
  for ( auto g : circ ) {
    switch (i) {
    case 0u:
      BOOST_CHECK( g.size() == 3u );
      break;
    case 1u:
      BOOST_CHECK( g.size() == 3u );
      break;
    case 2u:
      BOOST_CHECK( g.size() == 2u );
      break;
    case 3u:
      BOOST_CHECK( g.size() == 1u );
      break;
    }
    ++i;
  }
  BOOST_CHECK( i == 4u );
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
