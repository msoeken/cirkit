/* RevKit (www.revkit.org)
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
#define BOOST_TEST_MODULE esop_synthesis

#include <boost/test/unit_test.hpp>

#include <reversible/circuit.hpp>
#include <reversible/io/print_circuit.hpp>
#include <reversible/synthesis/esop_synthesis.hpp>

BOOST_AUTO_TEST_CASE(simple)
{
  using namespace cirkit;

  circuit circ;
  properties::ptr settings( new properties );
  settings->set( "negative_control_lines", true );
  esop_synthesis( circ, "../test/example.esop", settings );

  print_circuit( circ );
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
