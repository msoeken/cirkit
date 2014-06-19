/* RevKit: A Toolkit for Reversible Circuit Design (www.revkit.org)
 * Copyright (C) 2009-2014  The RevKit Developers <revkit@informatik.uni-bremen.de>
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
 * @file sequential_simulation.hpp
 *
 * @brief Sequential Simulation considering state inputs
 *
 * @author Mathias Soeken
 * @since  1.2
 */

#ifndef SEQUENTIAL_SIMULATION_HPP
#define SEQUENTIAL_SIMULATION_HPP

#include <map>

#include <boost/dynamic_bitset.hpp>
#include <boost/function.hpp>

#include <core/properties.hpp>

#include <reversible/circuit.hpp>
#include <reversible/truth_table.hpp>
#include <reversible/simulation/simulation.hpp>

namespace cirkit
{

  /**
   * @brief Functor which is called after a circuit is simulated combinationally
   *
   * If set, this functor is called in \ref revkit::sequential_simulation "sequential_simulation"
   * after every circuit simulation with a mapping of the state signals and the current output pattern
   * as parameter.
   *
   * @since  1.2
   */
  typedef boost::function<boost::dynamic_bitset<>(const std::map<std::string, boost::dynamic_bitset<> >&, const boost::dynamic_bitset<>&)> sequential_step_result_func;

  /**
   * @brief Sequential Simulation considering state inputs
   *
   * This algorithm performs a sequential simulation, i.e the partial simulation
   * function is called for a list of input pattern. For that purpose, all state
   * signals in the circuit are initially set to a value and then re-used for the
   * successive simulations, therefore simulation the sequential behavior.
   * This function can be used also with the create_simulation_pattern function by
   * reading a SIM file.
   *
   * @param outputs Output patterns
   * @param circ Circuit to be simulated
   * @param inputs List of input pattern. In this pattern only the values for all
   *               input signals and no constant values or state values are set.
   * @param settings <table border="0" width="100%">
   *   <tr>
   *     <td class="indexkey">Setting</td>
   *     <td class="indexkey">Type</td>
   *     <td class="indexkey">Default Value</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">initial_state</td>
   *     <td class="indexvalue">bitset_map</td>
   *     <td class="indexvalue">bitset_map()</td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">Initial state for sequential signals per state.</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">vcd_filename</td>
   *     <td class="indexvalue">std::string</td>
   *     <td class="indexvalue">""</td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">If not empty, a wave trace is written to that file-name.</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">step_result</td>
   *     <td class="indexvalue">\ref revkit::sequential_step_result_func "sequential_step_result_func"</td>
   *     <td class="indexvalue">\ref revkit::sequential_step_result_func "sequential_step_result_func()" <i>(empty)</i></td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">A functor called at every simulation together with current state and output pattern.</td>
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
   * @since  1.2
   */
  bool sequential_simulation( std::vector<boost::dynamic_bitset<> >& outputs, const circuit& circ, const std::vector<boost::dynamic_bitset<> >& inputs, properties::ptr settings = properties::ptr(), properties::ptr statistics = properties::ptr() );

  /**
   * @brief Functor for the sequential_simulation algorithm
   *
   * @param settings Settings (see sequential_simulation)
   * @param statistics Statistics (see sequential_simulation)
   *
   * @return A functor which complies with the multi_step_simulation_func interface
   *
   * @since  1.2
   */
  multi_step_simulation_func sequential_simulation_func( properties::ptr settings = properties::ptr( new properties() ), properties::ptr statistics = properties::ptr( new properties() ) );

}

#endif /* SEQUENTIAL_SIMULATION_HPP */

// Local Variables:
// c-basic-offset: 2
// End:
