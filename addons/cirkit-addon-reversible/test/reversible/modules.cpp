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
#define BOOST_TEST_MODULE modules

#include <memory>

#define timer timer_class
#include <boost/test/unit_test.hpp>
#undef timer

#include <reversible/circuit.hpp>
#include <reversible/functions/add_gates.hpp>
#include <reversible/functions/circuit_from_string.hpp>
#include <reversible/functions/flatten_circuit.hpp>
#include <reversible/io/print_circuit.hpp>

using namespace cirkit;

BOOST_AUTO_TEST_CASE(simple)
{
  /* This is an implementation of the simple ripple-carry adder in Fig. 4 of quant-ph/0410184v1
   *
   * This implementation has no code for constants, garbage, and names.
   */
  const auto maj = circuit_from_string( "t2 c b, t2 c a, t3 a b c" );
  const auto uma = circuit_from_string( "t3 a b c, t2 c a, t2 a b" );

  const auto n = 6u; /* adder  width */

  circuit adder( 2u * n + 2u );
  adder.add_module( "maj", std::make_shared<circuit>( maj ) );
  adder.add_module( "uma", std::make_shared<circuit>( uma ) );

  for ( auto i = 0u; i < 6u; ++i )
  {
    const auto offset = 2u * i;
    insert_module( adder, i, "uma", gate::control_container(), {offset, offset + 1u, offset + 2u} );
    insert_module( adder, i, "maj", gate::control_container(), {offset, offset + 1u, offset + 2u} );
  }

  insert_cnot( adder, n, 2u * n, 2u * n + 1 );

  std::cout << adder << std::endl << std::endl;

  circuit flatten;
  flatten_circuit( adder, flatten );

  std::cout << flatten << std::endl;
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
