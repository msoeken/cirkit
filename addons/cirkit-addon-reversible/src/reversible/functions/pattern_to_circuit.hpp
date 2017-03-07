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
 * @file pattern_to_circuit.hpp
 *
 * @brief Creates a circuit realization for a transposition
 *
 * @author Mathias Soeken
 * @author Laura Tague
 * @since  1.3
 */

#ifndef PATTERN_TO_CIRCUIT_HPP
#define PATTERN_TO_CIRCUIT_HPP

#include <boost/dynamic_bitset.hpp>

namespace cirkit
{
  class circuit;

/**
 * @brief Creates a circuit realization for a permutation of two patterns
 *
 * This function takes two lines from a reversible truth table (inputs -> pattern1)
 * (inputs -> pattern2) which need to be swapped,
 * and returns a circuit that will swap the two patterns assigment
 * and vice versa by leaving all other input and output assignments unaltered.
 *
 * @param circ An empty circuit that will be created. The number of lines of the circuit
 *             must coincide with the length of the input and output assignments.
 * @param inputs pattern1 Assignment
 * @param outputs pattern2 Assignment
 *
 * @since 2.0
 */
void pattern_to_circuit( circuit& circ, const boost::dynamic_bitset<>& pattern1, const boost::dynamic_bitset<>& pattern2);

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
