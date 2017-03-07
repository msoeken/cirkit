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
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
