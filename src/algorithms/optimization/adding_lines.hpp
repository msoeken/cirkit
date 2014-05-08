/* RevKit: A Toolkit for Reversible Circuit Design (www.revkit.org)
 * Copyright (C) 2009-2013  The RevKit Developers <revkit@informatik.uni-bremen.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

#include <core/circuit.hpp>
#include <core/properties.hpp>
#include <core/truth_table.hpp>

#include <algorithms/optimization/optimization.hpp>

namespace revkit
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
  optimization_func adding_lines_func( properties::ptr settings = properties::ptr( new properties() ), properties::ptr statistics = properties::ptr( new properties() ) );

}

#endif /* ADDING_LINES_HPP */

// Local Variables:
// c-basic-offset: 2
// End:
