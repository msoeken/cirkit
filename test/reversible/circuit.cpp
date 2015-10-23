/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
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
#define BOOST_TEST_MODULE circuit

#include <boost/assign/std/vector.hpp>
#include <boost/test/included/unit_test.hpp>

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
