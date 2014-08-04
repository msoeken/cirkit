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

#include <boost/assign/std/vector.hpp>

#include <reversible/circuit.hpp>
#include <reversible/functions/add_gates.hpp>

using namespace boost::assign;
using namespace cirkit;

int main( int argc, char ** argv )
{
  circuit circ( 5 );

  append_cnot( circ, 2, 3 );
  prepend_cnot( circ, 0, 1 );
  append_fredkin( circ )( make_var( 0 ), make_var( 1 ) )( 2, 4 );
  insert_cnot( circ, 2, 1, 2 );
  prepend_not( circ, 2 );

  std::vector<unsigned> controls;
  controls += 0,1,2,3;
  append_toffoli( circ, controls, 4 );
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
