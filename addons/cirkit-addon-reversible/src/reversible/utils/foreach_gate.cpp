/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
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

#include "foreach_gate.hpp"

#include <boost/assign/std/vector.hpp>
#include <boost/dynamic_bitset.hpp>

#include <core/utils/bitset_utils.hpp>

#include <reversible/functions/add_gates.hpp>

using namespace boost::assign;

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

/**
 * @param target     Target line
 * @param controls   Controls is a bit-vector of size n-1, where n is the number of lines.
 *                   It ignores the position of the target line and is adjusted by the
 *                   function.
 * @param polarities The size of this bit-vector corresponds to the number of 1's in controls.
 */
circuit create_gate_circuit( unsigned target, boost::dynamic_bitset<> controls, boost::dynamic_bitset<> polarities )
{
  circuit c( controls.size() + 1u );

  gate::control_container cont;

  /* bpos iterates over the 1's in controls, pos iterates over the positions in polarities */
  auto bpos = controls.find_first();
  auto pos = 0u;
  while ( bpos != boost::dynamic_bitset<>::npos )
  {
    auto line = bpos < target ? bpos : bpos + 1u;
    cont += make_var( line, polarities[pos] );
    bpos = controls.find_next( bpos );
    ++pos;
  }

  append_toffoli( c, cont, target );

  return c;
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

void foreach_gate( unsigned num_lines, bool with_negated, const std::function<void(const circuit&)>& func )
{
  assert( num_lines );

  /* iterate over all targets */
  for ( auto target = 0u; target < num_lines; ++target )
  {
    boost::dynamic_bitset<> controls( num_lines - 1u );

    do {
      auto count = controls.count();

      if ( !with_negated )
      {
        func( create_gate_circuit( target, controls, boost::dynamic_bitset<>( count, ( 1 << count ) - 1u ) ) );
      }
      else
      {
        boost::dynamic_bitset<> polarities( count );
        do {
          func( create_gate_circuit( target, controls, polarities ) );
          inc( polarities );
        } while ( !polarities.none() );
      }

      inc( controls );
    } while ( !controls.none() );
  }
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
