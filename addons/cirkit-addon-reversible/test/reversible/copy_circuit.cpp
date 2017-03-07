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
