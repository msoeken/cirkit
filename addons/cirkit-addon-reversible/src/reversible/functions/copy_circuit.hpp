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
 * @file copy_circuit.hpp
 *
 * @brief Copies a circuit
 *
 * @author Mathias Soeken
 * @since  1.0
 */

#ifndef COPY_CIRCUIT_HPP
#define COPY_CIRCUIT_HPP

#include <vector>

#include <reversible/circuit.hpp>

namespace cirkit
{

  /**
   * @brief Copies a circuit with all meta information
   *
   * This function creates a copy of the circuit \p src in \p dest
   * including all meta information as input and output names,
   * and also constant input and garbage output information.
   *
   * @param src  Source circuit
   * @param dest Destination circuit
   *
   * @since  1.0
   */
  void copy_circuit( const circuit& src, circuit& dest );

  /**
   * @brief Copies a circuit based on a line filter
   *
   * This function creates a copy of the circuit \p src in \p dest
   * but considers only the lines that have been specified in \p filter.
   * The caller of the function is reponsible that \p dest still contains
   * valid gates, e.g. by checking that all lines that are not in filter
   * are empty or only contain control lines.
   *
   * @param src  Source circuit
   * @param dest Destination circuit
   *
   * @since  2.0
   */
  void copy_circuit( const circuit& src, circuit& dest, const std::vector<unsigned>& filter );


}

#endif /* COPY_CIRCUIT_HPP */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
