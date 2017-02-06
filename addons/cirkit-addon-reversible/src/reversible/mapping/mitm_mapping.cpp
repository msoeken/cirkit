/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2016  EPFL
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

#include "mitm_mapping.hpp"

#include <core/utils/timer.hpp>
#include <reversible/pauli_tags.hpp>
#include <reversible/target_tags.hpp>
#include <reversible/functions/add_gates.hpp>
#include <reversible/functions/copy_metadata.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

circuit mitm_mapping( const circuit& src, const properties::ptr& settings, const properties::ptr& statistics )
{
  properties_timer t( statistics );
  
  circuit dest( src.lines() );
  copy_metadata( src, dest );

  for ( const auto& g : src )
  {
    /* assertions */
    assert( is_toffoli( g ) && g.controls().size() <= 2u );
    for ( const auto& c : g.controls() )
    {
      assert( c.polarity() );
    }

    const auto target = g.targets().front();
    switch ( g.controls().size() )
    {
    case 0u:
      append_pauli( dest, target, pauli_axis::X );
      break;
    case 1u:
      append_cnot( dest, g.controls().front(), target );
      break;
    case 2u:
      const auto c1 = g.controls()[0u].line();
      const auto c2 = g.controls()[1u].line();

      append_hadamard( dest, target );
      append_pauli( dest, c1, pauli_axis::Z, 4u );
      append_pauli( dest, c2, pauli_axis::Z, 4u );
      append_pauli( dest, target, pauli_axis::Z, 4u );
      append_cnot( dest, c2, c1 );
      append_cnot( dest, target, c2 );
      append_cnot( dest, c1, target );
      append_pauli( dest, c2, pauli_axis::Z, 4u, true );
      append_cnot( dest, c1, c2 );
      append_pauli( dest, c1, pauli_axis::Z, 4u, true );
      append_pauli( dest, c2, pauli_axis::Z, 4u, true );
      append_pauli( dest, target, pauli_axis::Z, 4u );
      append_cnot( dest, target, c2 );
      append_cnot( dest, c1, target );
      append_cnot( dest, c2, c1 );
      append_hadamard( dest, target );
      break;
    }
  }
  
  return dest;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
