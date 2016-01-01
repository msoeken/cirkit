/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2016  EPFL
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

#include "pattern_to_circuit.hpp"

#include <boost/assign/std/vector.hpp>

#include <reversible/gate.hpp>
#include <reversible/functions/add_gates.hpp>

using namespace boost::assign;

namespace cirkit
{

void pattern_to_circuit( circuit& circ, const boost::dynamic_bitset<>& pattern1, const boost::dynamic_bitset<>& pattern2 )
{
  assert( pattern1.size() == pattern2.size() );
  assert( circ.lines() == pattern1.size() );

  auto diff = pattern1 ^ pattern2;

  /* Patterns are equal */
  if ( diff.none() ) return;

  /* Find last position in diff and its polarity */
  auto last_pos = 0u;
  auto pos = diff.find_first();

  do
  {
    last_pos = pos;
    pos = diff.find_next( pos );
  } while ( pos != boost::dynamic_bitset<>::npos );

  bool last_pos_polarity = pattern1[last_pos];

  /* Build circuit */
  auto index = 0u;
  gate::control_container controls;
  pos = diff.find_first();
  do
  {
    if ( pos != last_pos )
    {
      /* first and last part */
      insert_cnot( circ, index, make_var( last_pos, last_pos_polarity ), pos );
      insert_cnot( circ, index, make_var( last_pos, last_pos_polarity ), pos );
      ++index;
    }
    pos = diff.find_next( pos );
  } while ( pos != boost::dynamic_bitset<>::npos );

  for ( unsigned pos = 0u; pos < pattern2.size(); ++pos )
  {
    if ( pos == last_pos ) continue;
    controls += make_var( pos, pattern2[pos] );
  }

  insert_toffoli( circ, index, controls, last_pos );
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
