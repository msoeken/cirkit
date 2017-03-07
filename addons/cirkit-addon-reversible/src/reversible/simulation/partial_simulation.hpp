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
 * @file partial_simulation.hpp
 *
 * @brief Simulation considering constant inputs and garbage outputs
 *
 * @author Mathias Soeken
 * @author Stefan Frehse
 * @since  1.0
 */

#ifndef PARTIAL_SIMULATION_HPP
#define PARTIAL_SIMULATION_HPP

#include <boost/dynamic_bitset.hpp>

#include <core/properties.hpp>

#include <reversible/simulation/simulation.hpp>

namespace cirkit
{

  class circuit;

  /**
   * @brief Simulation considering constant inputs and garbage outputs
   *
   * This simulation function does not require a complete input pattern.
   * Constant Lines are omitted and not counted in the pattern. If you
   * have a circuit with 5 lines for example and the 2 and 3 lines are
   * constant input lines, only 3 bits are accessed in this simulation, and
   * counted from 0, without taking the constant inputs into account. A valid
   * pattern would be 6 = 110 (MSB is left), assigning the first line to 0,
   * and the fourth and the fifth line to 1.
   *
   * The constant values are automatically inserted by the algorithm. The same
   * applies, when returning the output pattern. Thereby, the garbage lines are
   * omitted and thus, the indices in the output pattern may not correspond
   * to the indices in the circuit. However, the order is respected.
   *
   * @param output Output pattern
   * @param circ Circuit to be simulated
   * @param input Input pattern
   * @param settings <table border="0" width="100%">
   *   <tr>
   *     <td class="indexkey">Setting</td>
   *     <td class="indexkey">Type</td>
   *     <td class="indexkey">Default Value</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">simulation</td>
   *     <td class="indexvalue">simulation_func</td>
   *     <td class="indexvalue">\ref revkit::simple_simulation_func "simple_simulation_func()"</td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">After extending the input pattern by the constant values, the pattern is simulated using this algorithm.</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">keep_full_output</td>
   *     <td class="indexvalue">bool</td>
   *     <td class="indexvalue">false</td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">If true, then the garbage output lines are not omitted after simulation.</td>
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
   * @since  1.0
   */
  bool partial_simulation( boost::dynamic_bitset<>& output, const circuit& circ, const boost::dynamic_bitset<>& input,
                           properties::ptr settings = properties::ptr(),
                           properties::ptr statistics = properties::ptr() );

  /**
   * @brief Functor for the \ref revkit::partial_simulation "partial_simulation" algorithm
   *
   * @param settings Settings (see \ref revkit::partial_simulation "partial_simulation")
   * @param statistics Statistics (see \ref revkit::partial_simulation "partial_simulation")
   *
   * @return A functor which complies with the \ref revkit::simulation_func "simulation_func" interface
   *
   * @since  1.0
   */
  simulation_func partial_simulation_func( properties::ptr settings = std::make_shared<properties>(), properties::ptr statistics = std::make_shared<properties>() );

}

#endif /* PARTIAL_SIMULATION_HPP */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
