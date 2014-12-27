/* RevKit (www.revkit.org)
 * Copyright (C) 2009-2014  University of Bremen
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
