/* RevKit (www.rekit.org)
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

#include <iostream>

#include <reversible/circuit.hpp>
#include <reversible/functions/pattern_to_circuit.hpp>
#include <reversible/io/print_circuit.hpp>

using namespace cirkit;

int main( int argc, char ** argv )
{
  circuit circ;
  circ.set_lines( 4u );
  boost::dynamic_bitset<> p1( 4u, 2u ); // 0100
  boost::dynamic_bitset<> p2( 4u, 5u );  // 1010

  pattern_to_circuit( circ, p1, p2 );

  std::cout << circ << std::endl;

  return 0;
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
