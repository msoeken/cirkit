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

#include "maslov_mapping.hpp"

#include <core/utils/timer.hpp>
#include <reversible/gate.hpp>
#include <reversible/pauli_tags.hpp>
#include <reversible/target_tags.hpp>
#include <reversible/functions/add_gates.hpp>
#include <reversible/functions/add_line_to_circuit.hpp>
#include <reversible/functions/rewrite_circuit.hpp>
#include <reversible/mapping/mitm_mapping.hpp>
#include <reversible/utils/circuit_utils.hpp>

namespace cirkit
{

/******************************************************************************
 * private functions                                                          *
 ******************************************************************************/

unsigned find_first_empty_line( const gate& g )
{
  std::vector<unsigned> lines( g.size() );
  auto it = std::transform( g.controls().begin(), g.controls().end(), lines.begin(),
                            []( const variable& v ) { return v.line(); } );
  std::copy( g.targets().begin(), g.targets().end(), it );
  std::sort( lines.begin(), lines.end() );

  unsigned line = 0u;
  while ( line < lines.size() && lines[line] == line ) ++line;
  return line;
}

void append_srts( circuit& circ, unsigned a, unsigned b, unsigned c )
{
  append_hadamard( circ, c );
  append_cnot( circ, c, b );
  append_pauli( circ, b, pauli_axis::Z, 4u, true );
  append_cnot( circ, a, b );
  append_pauli( circ, b, pauli_axis::Z, 4u );
  append_cnot( circ, c, b );
  append_pauli( circ, b, pauli_axis::Z, 4u, true );
  append_cnot( circ, a, b );
  append_pauli( circ, b, pauli_axis::Z, 4u );
}

void append_srts_inv( circuit& circ, unsigned a, unsigned b, unsigned c )
{
  append_pauli( circ, b, pauli_axis::Z, 4u, true );
  append_cnot( circ, a, b );
  append_pauli( circ, b, pauli_axis::Z, 4u );
  append_cnot( circ, c, b );
  append_pauli( circ, b, pauli_axis::Z, 4u, true );
  append_cnot( circ, a, b );
  append_pauli( circ, b, pauli_axis::Z, 4u );
  append_cnot( circ, c, b );
  append_hadamard( circ, c );
}

void append_rts( circuit& circ, unsigned a, unsigned b, unsigned c )
{
  append_hadamard( circ, c );
  append_pauli( circ, c, pauli_axis::Z, 4u );
  append_cnot( circ, b, c );
  append_pauli( circ, c, pauli_axis::Z, 4u, true );
  append_cnot( circ, a, c );
}

void append_rts_inv( circuit& circ, unsigned a, unsigned b, unsigned c )
{
  append_cnot( circ, a, c );
  append_pauli( circ, c, pauli_axis::Z, 4u );
  append_cnot( circ, b, c );
  append_pauli( circ, c, pauli_axis::Z, 4u, true );
  append_hadamard( circ, c );
}

void append_rtl4( circuit& circ, unsigned a, unsigned b, unsigned c, unsigned d )
{
  append_hadamard( circ, d );
  append_pauli( circ, d, pauli_axis::Z, 4u );
  append_cnot( circ, c, d );
  append_pauli( circ, d, pauli_axis::Z, 4u, true );
  append_hadamard( circ, d );
  append_cnot( circ, a, d );
  append_pauli( circ, d, pauli_axis::Z, 4u );
  append_cnot( circ, b, d );
  append_pauli( circ, d, pauli_axis::Z, 4u, true );
  append_cnot( circ, a, d );
  append_pauli( circ, d, pauli_axis::Z, 4u );
  append_cnot( circ, b, d );
  append_pauli( circ, d, pauli_axis::Z, 4u, true );
  append_hadamard( circ, d );
  append_pauli( circ, d, pauli_axis::Z, 4u );
  append_cnot( circ, c, d );
  append_pauli( circ, d, pauli_axis::Z, 4u, true );
  append_hadamard( circ, d );
}

void append_rtl4_inv( circuit& circ, unsigned a, unsigned b, unsigned c, unsigned d )
{
  append_hadamard( circ, d );
  append_pauli( circ, d, pauli_axis::Z, 4u );
  append_cnot( circ, c, d );
  append_pauli( circ, d, pauli_axis::Z, 4u, true );
  append_hadamard( circ, d );
  append_pauli( circ, d, pauli_axis::Z, 4u );
  append_cnot( circ, b, d );
  append_pauli( circ, d, pauli_axis::Z, 4u, true );
  append_cnot( circ, a, d );
  append_pauli( circ, d, pauli_axis::Z, 4u );
  append_cnot( circ, b, d );
  append_pauli( circ, d, pauli_axis::Z, 4u, true );
  append_cnot( circ, a, d );
  append_hadamard( circ, d );
  append_pauli( circ, d, pauli_axis::Z, 4u );
  append_cnot( circ, c, d );
  append_pauli( circ, d, pauli_axis::Z, 4u, true );
  append_hadamard( circ, d );
}

/******************************************************************************
 * public functions                                                           *
 ******************************************************************************/

circuit maslov_mapping( const circuit& src, const properties::ptr& settings, const properties::ptr& statistics )
{
  properties_timer t( statistics );

  auto circ = rewrite_circuit( src, {
      []( const gate& g, circuit& dest ) {
        if ( !is_toffoli( g ) ) return false;
        if ( has_negative_control( g ) ) return false;

        const auto target = g.targets().front();

        switch ( g.controls().size() )
        {
        default: return false;

        case 0u:
          append_pauli( dest, target, pauli_axis::X );
          break;

        case 1u:
          append_cnot( dest, g.controls().front(), target );
          break;

        case 2u:
          append_mitm( dest, g.controls()[0u].line(), g.controls()[1u].line(), target );
          break;

        case 3u:
          {
            const auto c1 = g.controls()[0u].line();
            const auto c2 = g.controls()[1u].line();
            const auto c3 = g.controls()[2u].line();
            const auto hl = find_first_empty_line( g );

            // R1-TOF(c1, c2, hl)
            append_hadamard( dest, hl );
            append_pauli( dest, hl, pauli_axis::Z, 4u );
            append_cnot( dest, c2, hl );
            append_pauli( dest, hl, pauli_axis::Z, 4u, true );
            append_cnot( dest, c1, hl );
            append_pauli( dest, hl, pauli_axis::Z, 4u );
            append_cnot( dest, c2, hl );
            append_pauli( dest, hl, pauli_axis::Z, 4u, true );
            append_hadamard( dest, hl );

            // S-R2-TOF(c3, hl, target)
            append_srts( dest, c3, hl, target );

            // R1-TOF^-1(c1, c2, hl)
            append_hadamard( dest, hl );
            append_pauli( dest, hl, pauli_axis::Z, 4u );
            append_cnot( dest, c2, hl );
            append_pauli( dest, hl, pauli_axis::Z, 4u, true );
            append_cnot( dest, c1, hl );
            append_pauli( dest, hl, pauli_axis::Z, 4u );
            append_cnot( dest, c2, hl );
            append_pauli( dest, hl, pauli_axis::Z, 4u, true );
            append_hadamard( dest, hl );

            // S-R2-TOF^-1(c3, hl, target)
            append_srts_inv( dest, c3, hl, target );
          } break;

        case 4u:
          {
            const auto c1 = g.controls()[0u].line();
            const auto c2 = g.controls()[1u].line();
            const auto c3 = g.controls()[2u].line();
            const auto c4 = g.controls()[3u].line();
            const auto hl = find_first_empty_line( g );

            append_rtl4( dest, c1, c2, c3, hl );
            append_srts( dest, c4, hl, target );
            append_rtl4_inv( dest, c1, c2, c3, hl );
            append_srts_inv( dest, c4, hl, target );
          } break;

        case 5u:
          {
            const auto c1 = g.controls()[0u].line();
            const auto c2 = g.controls()[1u].line();
            const auto c3 = g.controls()[2u].line();
            const auto c4 = g.controls()[3u].line();
            const auto c5 = g.controls()[4u].line();
            const auto hl1 = find_first_empty_line( g );
            const auto hl2 = hl1 + 1u; // TODO

            append_srts( dest, c5, hl2, target );
            append_rts( dest, hl1, c4, hl2 );
            append_rtl4( dest, c1, c2, c3, hl1 );
            append_rts_inv( dest, hl1, c4, hl2 );

            append_srts_inv( dest, c5, hl2, target );
            append_rts( dest, hl1, c4, hl2 );
            append_rtl4_inv( dest, c1, c2, c3, hl1 );
            append_rts_inv( dest, hl1, c4, hl2 );
          } break;
        }

        return true;
      }
    } );

  if ( has_fully_controlled_gate( src ) && src.lines() > 3u )
  {
    add_line_to_circuit( circ, "h", "h", false, true );

    if ( src.lines() == 6u ) /* TODO */
    {
      add_line_to_circuit( circ, "h", "h", false, true );
    }
  }

  return circ;
}


}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
