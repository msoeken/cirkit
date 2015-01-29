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
#define BOOST_TEST_MODULE circuit_io

#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp>

#include <reversible/circuit.hpp>
#include <reversible/functions/add_gates.hpp>
#include <reversible/io/print_circuit.hpp>
#include <reversible/io/read_realization.hpp>
#include <reversible/io/write_realization.hpp>
#include <reversible/io/write_verilog.hpp>

BOOST_AUTO_TEST_CASE(simple)
{
  using boost::test_tools::output_test_stream;

  using namespace cirkit;

  circuit circ( 3u ), circ2;
  std::string circ_str = "―⊕―○―――●―\n―――⊕―●―⨯―\n―○―●―⊕―⨯―\n";
  output_test_stream output, output2;

  /* create circuit */
  append_cnot( circ, make_var( 2u, false ), 0u );
  append_toffoli( circ )( make_var( 0u, false ), make_var( 2u ) )( 1u );
  append_cnot( circ, 1u, 2u );
  append_fredkin( circ )( make_var( 0u ) )( 1u, 2u );

  /* print circuit */
  output << circ;
  BOOST_CHECK( output.is_equal( circ_str ) );

  /* write and read circuit */
  write_realization( circ, "/tmp/test.real" );
  read_realization( circ2, "/tmp/test.real" );

  /* print read circuit */
  output2 << circ;
  BOOST_CHECK( output2.is_equal( circ_str ) );

  /* write verilog */
  write_verilog( circ, "/tmp/test.v" );
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
