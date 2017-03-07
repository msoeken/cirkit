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
 * @file add_circuit.hpp
 *
 * @brief Prepending, inserting and appending circuits to another circuit
 *
 * @author Mathias Soeken
 * @since  1.0
 */

#include <vector>

#include <reversible/circuit.hpp>

namespace cirkit
{

/**
 * @brief Insert a circuit \p src at the end of another circuit \p circ
 *
 * @param circ Destination circuit
 * @param src  Source circuit
 * @param controls Controls, which are added to each gate in \p src (introduced in version 1.1)
 * @param line_map If not empty, maps lines from src to circ according to map
 *
 * @since  1.0
 */
void append_circuit( circuit& circ, const circuit& src, const gate::control_container& controls = gate::control_container(), const std::vector<unsigned>& line_map = std::vector<unsigned>() );

/**
 * @brief Insert a circuit \p src at the beginning of another circuit \p circ
 *
 * @param circ Destination circuit
 * @param src  Source circuit
 * @param controls Controls, which are added to each gate in \p src (introduced in version 1.1)
 * @param line_map If not empty, maps lines from src to circ according to map
 *
 * @since  1.0
 */
void prepend_circuit( circuit& circ, const circuit& src, const gate::control_container& controls = gate::control_container(), const std::vector<unsigned>& line_map = std::vector<unsigned>() );

/**
 * @brief Insert a circuit \p src before gate \p pos (counting from 0) of another circuit \p circ
 *
 * @param circ Destination circuit
 * @param pos  Position where to insert
 * @param src  Source circuit
 * @param controls Controls, which are added to each gate in \p src (introduced in version 1.1)
 * @param line_map If not empty, maps lines from src to circ according to map
 *
 * @since  1.0
 */
void insert_circuit( circuit& circ, unsigned pos, const circuit& src, const gate::control_container& controls = gate::control_container(), const std::vector<unsigned>& line_map = std::vector<unsigned>() );

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
