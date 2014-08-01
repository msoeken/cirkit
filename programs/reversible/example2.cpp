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
#include <reversible/functions/reverse_circuit.hpp>
#include <reversible/io/read_realization.hpp>
#include <reversible/io/write_realization.hpp>

using namespace cirkit;

int main( int argc, char ** argv )
{
  circuit circ;
  read_realization( circ, "circuit.real" );
  reverse_circuit( circ );
  write_realization( circ, "circuit-copy.real" );

  return 0;
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
