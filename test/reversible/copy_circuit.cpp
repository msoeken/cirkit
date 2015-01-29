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
#define BOOST_TEST_MODULE truth_table

#include <boost/assign/std/vector.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp>

#include <reversible/circuit.hpp>
#include <reversible/functions/add_gates.hpp>
#include <reversible/functions/copy_circuit.hpp>
#include <reversible/io/print_circuit.hpp>

BOOST_AUTO_TEST_CASE(simple)
{
  using boost::test_tools::output_test_stream;

  using namespace boost::assign;
  using namespace cirkit;

  circuit circ( 6u ), copy, smaller_copy;

  append_toffoli( circ )( 0u, 2u )( 5u );
  append_cnot( circ, 0u, 2u );
  append_not( circ, 0u );

  output_test_stream output;
  output << circ;

  BOOST_CHECK( !output.is_empty( false ) );
  BOOST_CHECK( output.is_equal( "**O\n---\n*O-\n---\n---\nO--\n" ) );

  copy_circuit( circ, copy );

  output << copy;

  BOOST_CHECK( !output.is_empty( false ) );
  BOOST_CHECK( output.is_equal( "**O\n---\n*O-\n---\n---\nO--\n" ) );

  std::vector<unsigned> filter;
  filter += 0u,2u,5u;

  copy_circuit( circ, smaller_copy, filter );

  output << smaller_copy;

  BOOST_CHECK( !output.is_empty( false ) );
  BOOST_CHECK( output.is_equal( "**O\n*O-\nO--\n" ) );
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
