/* RevKit: A Toolkit for Reversible Circuit Design (www.revkit.org)
 * Copyright (C) 2009-2014  The RevKit Developers <revkit@informatik.uni-bremen.de>
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
#include <reversible/functions/add_gates.hpp>
#include <reversible/io/print_circuit.hpp>

using namespace cirkit;

int main( int argc, char ** argv )
{
  circuit circ( 3 );

  append_toffoli( circ )( 2u )( 1 );
  append_toffoli( circ )( 0u, 1u )( 2 );
  append_toffoli( circ )( 1u, 2u )( 0 );
  append_toffoli( circ )( 0u, 1u )( 2 );
  append_toffoli( circ )( 2u )( 1 );

  std::cout << circ << std::endl;

  return 0;
}
