/* RevKit (www.rekit.org)
 * Copyright (C) 2009-2014  University of Bremen
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
  simulation_func partial_simulation_func( properties::ptr settings = properties::ptr( new properties() ), properties::ptr statistics = properties::ptr( new properties() ) );

}

#endif /* PARTIAL_SIMULATION_HPP */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
