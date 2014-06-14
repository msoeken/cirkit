/* RevKit: A Toolkit for Reversible Circuit Design (www.revkit.org)
 * Copyright (C) 2009-2013  The RevKit Developers <revkit@informatik.uni-bremen.de>
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

#include "transposition_to_circuit.hpp"

#include <boost/assign/std/vector.hpp>

#include "../circuit.hpp"
#include "../gate.hpp"

#include "add_circuit.hpp"
#include "add_gates.hpp"
#include "reverse_circuit.hpp"

using namespace boost::assign;

namespace cirkit
{
  bool transposition_to_circuit( circuit& circ, const boost::dynamic_bitset<>& inputs, const boost::dynamic_bitset<>& outputs )
  {
    assert( inputs.size() == outputs.size() );
    assert( circ.lines() == inputs.size() );

    unsigned b = 0u, bs = 0u;
    circuit circ_block_a( inputs.size() );
    circuit circ_block_b( inputs.size() );
    circuit circ_block_bs( inputs.size() );
    circuit circ_block_c( inputs.size() );
    circuit circ_block_X( inputs.size() );
    append_not( circ_block_X, 0u );
    gate::control_container controls;
    unsigned target;

    for ( unsigned j = 0u; j < inputs.size(); ++j )
    {
      target = inputs.size() - 1u - j;
      if ( !inputs.test( j ) && !outputs.test( j ) )
      {
        append_not( circ_block_a, target );
      }
      else if ( inputs.test( j ) && !outputs.test( j ) )
      {
        controls.clear();
        append_not( circ_block_b, target );
        ++b;
        append_not( circ_block_c, target );
        for ( unsigned k = 0u; k < inputs.size(); ++k )
        {
          if ( k != target )
          {
            controls += make_var( k );
          }
        }
        append_toffoli( circ_block_c, controls, target );
        circ_block_X.remove_gate_at( 0u );
        append_toffoli( circ_block_X, controls, target );
      }
      else if ( !inputs.test( j ) && outputs.test( j ) )
      {
        controls.clear();
        append_not( circ_block_bs, target );
        ++bs;
        append_not( circ_block_c, target );
        for ( unsigned k = 0u; k < inputs.size(); ++k )
        {
          if ( k != target )
          {
            controls += make_var( k );
          }
        }
        append_toffoli( circ_block_c, controls, target );
        circ_block_X.remove_gate_at( 0u );
        append_toffoli( circ_block_X, controls, target );
      }
    }

    circ_block_c.remove_gate_at( circ_block_c.num_gates() - 1 );
    circ_block_c.remove_gate_at( circ_block_c.num_gates() - 1 );

    append_circuit( circ, circ_block_a );
    if ( b < bs )
    {
      append_circuit( circ, circ_block_b );
    }
    else
    {
      append_circuit( circ, circ_block_bs );
    }
    append_circuit( circ, circ_block_c );

    append_circuit( circ, circ_block_X );

    reverse_circuit( circ_block_c );
    append_circuit( circ, circ_block_c );

    if ( b < bs )
    {
      append_circuit( circ, circ_block_b );
    }
    else
    {
      append_circuit( circ, circ_block_bs );
    }
    append_circuit( circ, circ_block_a );

    return true;
  }
}

// Local Variables:
// c-basic-offset: 2
// End:
