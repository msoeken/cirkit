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
 * @file add_line_to_circuit.hpp
 *
 * @brief Add a line to a circuit with specifying all meta-data
 *
 * @author Mathias Soeken
 * @since  1.0
 */

#ifndef ADD_LINE_TO_CIRCUIT_HPP
#define ADD_LINE_TO_CIRCUIT_HPP

#include <string>

#include <reversible/circuit.hpp>

namespace cirkit
{

  /**
   * @brief Add a line to a circuit with specifying all meta-data
   *
   * This function helps adding a line to the circuit.
   * Besides incrementing the line counter, all meta-data information
   * is adjusted as well.
   *
   * @param circ Circuit
   * @param input Name of the input of the line
   * @param output Name of the output of the line
   * @param c Constant value of that line (Default: Not constant)
   * @param g If true, line is a garbage line
   *
   * @return The index of the newly added line
   *
   * @since  1.0 (Return value since 1.1)
   */
  unsigned add_line_to_circuit( circuit& circ, const std::string& input, const std::string& output, const constant& c = constant(), bool g = false );

}

#endif /* ADD_LINE_TO_CIRCUIT_HPP */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
