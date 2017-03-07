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
 * @file expand_circuit.hpp
 *
 * @brief Expand a circuit on the base of a sub circuit
 *
 * @author Mathias Soeken
 * @since  1.0
 */

#ifndef EXPAND_CIRCUIT_HPP
#define EXPAND_CIRCUIT_HPP

#include <vector>

#include <reversible/circuit.hpp>

namespace cirkit
{

  /**
   * @brief Expands a circuit
   *
   * This function expands the circuit \p base, such that
   * it will have \p num_lines and maps each line \em i
   * in the circuit \p base to the line \em filter[i] in the
   * circuit \p circ.
   *
   * @param base Base circuit
   * @param circ Newly created circuit, extended from \p base. Needs to be empty.
   * @param num_lines New number of lines
   * @param filter Mapping for calculating the new line indices.
   *
   * @return true on success, false otherwise
   *
   * @since  1.0
   */
  bool expand_circuit( const circuit& base, circuit& circ, unsigned num_lines, const std::vector<unsigned>& filter );

}

#endif /* EXPAND_CIRCUIT_HPP */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
