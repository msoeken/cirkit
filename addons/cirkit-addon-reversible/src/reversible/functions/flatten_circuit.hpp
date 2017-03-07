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
 * @file flatten_circuit.hpp
 *
 * @brief Flattens a circuit with modules
 *
 * @author Mathias Soeken
 * @since  1.1
 */

#ifndef FLATTEN_CIRCUIT_HPP
#define FLATTEN_CIRCUIT_HPP

namespace cirkit
{

  class circuit;

  /**
   * @brief Flattens a circuit with modules
   *
   * This functions takes a circuit with module \p base and
   * substitutes all modules with their elementary gates
   * recursively and saves the result in \p circ.
   *
   * @param base Circuit to flatten (with modules)
   * @param circ Resulting circuit (without modules)
   * @param keep_meta_data Specifies, whether the RevLib 2.0 meta data such as buses should be kept (since 1.2)
   *
   * @since  1.1
   */
  void flatten_circuit( const circuit& base, circuit& circ, bool keep_meta_data = false );

}

#endif /* FLATTEN_CIRCUIT_HPP */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
