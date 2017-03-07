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
 * @file circuit_to_truth_table.hpp
 *
 * @brief Generates a truth table from a circuit
 *
 * @author Mathias Soeken
 * @since  1.0
 */

#ifndef CIRCUIT_TO_TRUTH_TABLE_HPP
#define CIRCUIT_TO_TRUTH_TABLE_HPP

#include <boost/dynamic_bitset.hpp>
#include <boost/function.hpp>

#include <core/functor.hpp>
#include <reversible/circuit.hpp>
#include <reversible/truth_table.hpp>

namespace cirkit
{

  /**
   * @brief Generates a truth table from a circuit
   *
   * This function takes a circuit, simulates it with a custom simulation function
   * and creates the specification. Further, the meta is copied.
   *
   * @param circ Circuit to be simulated
   * @param spec Empty truth table to be constructed
   * @param simulation Simulation function object
   *
   * @return true on success, false otherwise
   *
   * @since  1.0
   */
  bool circuit_to_truth_table( const circuit& circ, binary_truth_table& spec, const functor<bool(boost::dynamic_bitset<>&, const circuit&, const boost::dynamic_bitset<>&)>& simulation );

}

#endif /* CIRCUIT_TO_TRUTH_TABLE_HPP */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
