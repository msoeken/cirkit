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
 * @file transposition_to_circuit.hpp
 *
 * @brief Creates a circuit realization for a transposition
 *
 * @author Mathias Soeken
 * @author Nils Przigoda
 * @since  1.3
 */
#ifndef TRANSPOSITION_TO_CIRCUIT
#define TRANSPOSITION_TO_CIRCUIT

#include <boost/dynamic_bitset.hpp>

namespace cirkit
{
  class circuit;

  /**
   * @brief Creates a circuit realization for a transposition
   *
   * This function takes one line from a reversible truth table (inputs -> outputs)
   * and returns a circuit that will map this input assignment to the output assignment
   * and vice versa by leaving all other input and output assignments unaltered.
   *
   * @param circ An empty circuit that will be created. The number of lines of the circuit
   *             must coincide with the length of the input and output assignments.
   * @param inputs Input Assignment
   * @param outputs Output Assignment
   *
   * @since  1.3
   */
  bool transposition_to_circuit( circuit& circ,
      const boost::dynamic_bitset<>& inputs,
      const boost::dynamic_bitset<>& outputs);

}

#endif /* TRANSPOSITION_TO_CIRCUIT */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
