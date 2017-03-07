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
#define BOOST_TEST_MODULE change_polarity

#include <iostream>

#include <boost/range/algorithm.hpp>
#include <boost/test/unit_test.hpp>

#include <reversible/circuit.hpp>
#include <reversible/functions/add_gates.hpp>
#include <reversible/io/print_circuit.hpp>

using namespace cirkit;

BOOST_AUTO_TEST_CASE(simple)
{

  circuit c( 3 );
  append_toffoli( c )( 0u, 1u )( 2u );
  append_cnot( c, 0u, 1u );
  append_not( c, 0u );

  std::cout << "[i] before:" << std::endl << c << std::endl;

  /* find CNOT gate */
  auto it = boost::find_if( c, []( const gate& g ) { return g.controls().size() == 1u; } );
  if ( it != c.end() )
  {
    /* change polarity and line */
    it->controls()[0u].set_line( 2u );
    it->controls()[0u].set_polarity( false );
  }

  std::cout << "[i] after:" << std::endl << c << std::endl;
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
