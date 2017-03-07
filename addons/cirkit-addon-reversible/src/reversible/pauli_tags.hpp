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

/**
 * @file pauli_tags.hpp
 *
 * @brief Pauli(-root) gates
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef PAULI_TAGS_HPP
#define PAULI_TAGS_HPP

#include <reversible/circuit.hpp>
#include <reversible/gate.hpp>

namespace cirkit
{

enum class pauli_axis { X, Y, Z };

/**
 * @brief Target tag for Pauli gates
 */
struct pauli_tag
{
  pauli_tag() {}

  pauli_tag( pauli_axis axis, unsigned root = 1u, bool adjoint = false )
    : axis( axis ),
      root( root ),
      adjoint( adjoint )
  {
  }

  /**
   * @brief Axis of Pauli gate
   */
  pauli_axis axis = pauli_axis::X;

  /**
   * @brief root, i.e., U^{1/root}
   */
  unsigned root = 1u;

  /**
   * @brief Sets whether it's the adjoint (dagger)
   */
  bool adjoint = false;
};

bool is_pauli( const gate& g );

gate& append_pauli( circuit& circ, unsigned target, pauli_axis axis, unsigned root = 1u, bool adjoint = false );
gate& prepend_pauli( circuit& circ, unsigned target, pauli_axis axis, unsigned root = 1u, bool adjoint = false );
gate& insert_pauli( circuit& circ, unsigned n, unsigned target, pauli_axis axis, unsigned root = 1u, bool adjoint = false );

struct hadamard_tag {};

bool is_hadamard( const gate& g );

gate& append_hadamard( circuit& circ, unsigned target );
gate& prepend_hadamard( circuit& circ, unsigned target );
gate& insert_hadamard( circuit& circ, unsigned n, unsigned target );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
