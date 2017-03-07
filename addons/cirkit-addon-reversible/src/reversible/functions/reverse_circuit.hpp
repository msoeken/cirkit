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
 * @file reverse_circuit.hpp
 *
 * @brief Reverse a circuit
 *
 * @author Mathias Soeken
 * @since  1.0
 */

#ifndef REVERSE_CIRCUIT_HPP
#define REVERSE_CIRCUIT_HPP

#include <reversible/circuit.hpp>

namespace cirkit
{

  /**
   * @brief Reverse a circuit
   *
   * This function reverses a circuit \p src and writes
   * the result to a new circuit \p dest.
   *
   * You can use reverse_circuit(circuit&) if the circuit
   * should be reversed in-place.
   *
   * @param src  Source circuit
   * @param dest Destination circuit
   *
   * @since  1.0
   */
  void reverse_circuit( const circuit& src, circuit& dest );

  /**
   * @brief Reverse a circuit in-place
   *
   * This function reverses a circuit \p circ in-place.
   *
   * @param circ Circuit to be reversed
   *
   * @since  1.0
   */
  void reverse_circuit( circuit& circ );

}

#endif /* REVERSE_CIRCUIT_HPP */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
