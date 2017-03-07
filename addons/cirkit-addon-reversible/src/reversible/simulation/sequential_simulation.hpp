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
  multi_step_simulation_func sequential_simulation_func( properties::ptr settings = std::make_shared<properties>(), properties::ptr statistics = std::make_shared<properties>() );

}

#endif /* SEQUENTIAL_SIMULATION_HPP */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
