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
 * @file adding_lines.hpp
 *
 * @brief Adding Lines Optimization
 *
 * @author Mathias Soeken
 * @author Robert Wille
 * @since  1.1
 */

#ifndef ADDING_LINES_HPP
#define ADDING_LINES_HPP

#include <core/properties.hpp>

#include <reversible/circuit.hpp>
#include <reversible/truth_table.hpp>

#include <reversible/optimization/optimization.hpp>

namespace cirkit
{

  /**
   * @brief Adding Lines Optimization
   *
   * This algorithm implements the Adding Lines Optimization proposed in \ref MWD10 "[MWD10]" which
   * adds helper lines to reduce quantum costs in reversible circuits.
   *
   * @param circ Optimized circuit to be generated
   * @param base Original circuit
   * @param settings <table border="0" width="100%">
   *   <tr>
   *     <td class="indexkey">Setting</td>
   *     <td class="indexkey">Type</td>
   *     <td class="indexkey">Default Value</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">additional_lines</td>
   *     <td class="indexvalue">unsigned</td>
   *     <td class="indexvalue">1u</td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">Number of additional helper lines to add to the circuit.</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">cost_function</td>
   *     <td class="indexvalue">\ref revkit::cost_function "cost_function"</td>
   *     <td class="indexvalue">\ref revkit::gate_costs "costs_by_gate_func( transistor_costs() )"</td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">Cost function to determine whether the optimized circuit is cheaper.</td>
   *   </tr>
   * </table>
   * @param statistics <table border="0" width="100%">
   *   <tr>
   *     <td class="indexkey">Information</td>
   *     <td class="indexkey">Type</td>
   *     <td class="indexkey">Description</td>
   *   </tr>
   *   <tr>
   *     <td class="indexvalue">runtime</td>
   *     <td class="indexvalue">double</td>
   *     <td class="indexvalue">Run-time consumed by the algorithm in CPU seconds.</td>
   *   </tr>
   * </table>
   * @return true on success
   *
   * @since  1.1
   */
  bool adding_lines( circuit& circ, const circuit& base, properties::ptr settings = properties::ptr(), properties::ptr statistics = properties::ptr() );

  /**
   * @brief Functor for the \ref revkit::adding_lines "adding_lines" algorithm
   *
   * @param settings Settings (see \ref revkit::adding_lines "adding_lines")
   * @param statistics Statistics (see \ref revkit::adding_lines "adding_lines")
   *
   * @return A functor which complies with the \ref revkit::optimization_func "optimization_func" interface
   *
   * @since  1.1
   */
  optimization_func adding_lines_func( properties::ptr settings = std::make_shared<properties>(), properties::ptr statistics = std::make_shared<properties>() );

}

#endif /* ADDING_LINES_HPP */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
