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
