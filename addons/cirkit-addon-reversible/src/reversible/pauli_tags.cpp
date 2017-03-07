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

#include "pauli_tags.hpp"

#include <reversible/target_tags.hpp>

namespace cirkit
{

bool is_pauli( const gate& g )
{
  return is_type<pauli_tag>( g.type() );
}

gate& create_pauli( gate& g, unsigned target, pauli_axis axis, unsigned root, bool adjoint )
{
  g.add_target( target );
  g.set_type( pauli_tag( axis, root, adjoint ) );
  return g;
}

gate& append_pauli( circuit& circ, unsigned target, pauli_axis axis, unsigned root, bool adjoint )
{
  return create_pauli( circ.append_gate(), target, axis, root, adjoint );
}

gate& prepend_pauli( circuit& circ, unsigned target, pauli_axis axis, unsigned root, bool adjoint )
{
  return create_pauli( circ.prepend_gate(), target, axis, root, adjoint );
}

gate& insert_pauli( circuit& circ, unsigned n, unsigned target, pauli_axis axis, unsigned root, bool adjoint )
{
  return create_pauli( circ.insert_gate( n ), target, axis, root, adjoint );
}

bool is_hadamard( const gate& g )
{
  return is_type<hadamard_tag>( g.type() );
}

gate& create_hadamard( gate& g, unsigned target )
{
  g.add_target( target );
  g.set_type( hadamard_tag() );
  return g;
}

gate& append_hadamard( circuit& circ, unsigned target )
{
  return create_hadamard( circ.append_gate(), target );
}

gate& prepend_hadamard( circuit& circ, unsigned target )
{
  return create_hadamard( circ.prepend_gate(), target );
}

gate& insert_hadamard( circuit& circ, unsigned n, unsigned target )
{
  return create_hadamard( circ.insert_gate( n ), target );
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
