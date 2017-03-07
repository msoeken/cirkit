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
