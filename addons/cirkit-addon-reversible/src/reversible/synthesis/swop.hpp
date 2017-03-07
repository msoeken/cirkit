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
 * @file swop.hpp
 *
 * @brief SWOP - Synthesis With Output Permutation
 *
 * @author Mathias Soeken
 * @since  1.0
 */

#ifndef SWOP_HPP
#define SWOP_HPP

#include <boost/scoped_ptr.hpp>

#include <core/utils/timer.hpp>
#include <core/properties.hpp>

#include <reversible/circuit.hpp>
#include <reversible/truth_table.hpp>
#include <reversible/functions/clear_circuit.hpp>
#include <reversible/functions/copy_circuit.hpp>
#include <reversible/utils/costs.hpp>

#include <reversible/synthesis/synthesis.hpp>

namespace cirkit
{

  /**
   * @brief Functor which can be called after each SWOP iteration
   *
   * This functor is used for the \em stepfunc setting in the
   * \ref revkit::swop "swop" synthesis algorithm. It takes no arguments
   * and returns no value.
   *
   * @since  1.0
   */
  typedef std::function<void()> swop_step_func;

  /**
   * @brief SWOP Synthesis Approach
   *
   * This is an implementation of the SWOP (Synthesis with Output Permutation) synthesis approach as introduced in [\ref WGDD09].
   * Thereby it is generic and can be used with every truth table based synthesis approach, which gets a circuit and
   * a truth table as parameters.
   *
   * @section sec_example_swop Example
   * This example makes a fitting operator from the transformation_based_synthesis function, taking
   * two parameters, the circuit and the truth table.
   * @code
   * properties::ptr tbs_settings( new properties() );
   * // initialize settings for transformation based synthesis
   * properties::ptr tbs_statistics( new properties() );
   *
   * circuit circ;
   * binary_truth_table spec = // obtained from somewhere
   *
   * properties::ptr settings( new properties() ); // swop settings
   * settings->set( "synthesis", transformation_based_synthesis_func( settings, statistics ) );
   * swop( circ, spec, settings );
   * @endcode
   *
   * @param circ Circuit, must be empty, is filled with gate by the algorithm
   * @param spec Truth table used for synthesis
   * @param settings <table border="0" width="100%">
   *   <tr>
   *     <td class="indexkey">Setting</td>
   *     <td class="indexkey">Type</td>
   *     <td class="indexkey">Default Value</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">enable</td>
   *     <td class="indexvalue">bool</td>
   *     <td class="indexvalue">true</td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">This setting enables the SWOP algorithm to do swapping of the outputs at all. This is useful, when using SWOP as an additional option to a synthesis approach, only one call is necessary, i.e. using swop with the respective synthesis approach but disabling the swapping. Then the algorithm behaves as called stand-alone.</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">exhaustive</td>
   *     <td class="indexvalue">bool</td>
   *     <td class="indexvalue">false</td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">When set to true, all possible permutations are checked, otherwise sifting is used to find a permutation, which may not be optimal.</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">synthesis</td>
   *     <td class="indexvalue">\ref revkit::truth_table_synthesis_func "truth_table_synthesis_func"</td>
   *     <td class="indexvalue">\ref revkit::transformation_based_synthesis_func "transformation_based_synthesis_func()"</td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">Synthesis function to be used with the SWOP algorithm.</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">cost_function</td>
   *     <td class="indexvalue">\ref revkit::cost_function "cost_function"</td>
   *     <td class="indexvalue">\ref revkit::gate_costs "costs_by_circuit_func( gate_costs() )"</td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">Cost function to determine which circuit to use.</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">stepfunc</td>
   *     <td class="indexvalue">\ref revkit::swop_step_func "swop_step_func"</td>
   *     <td class="indexvalue">\ref revkit::swop_step_func "swop_step_func()" <i>Empty functor</i></td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">This functor is called after each iteration.</td>
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
   * @since 1.0
   */
  bool swop( circuit& circ, const binary_truth_table& spec,
             properties::ptr settings = properties::ptr(),
             properties::ptr statistics = properties::ptr() );

  /**
   * @brief Functor for the \ref revkit::swop "swop" algorithm
   *
   * @param settings Settings (see \ref revkit::swop "swop")
   * @param statistics Statistics (see \ref revkit::swop "swop")
   *
   * @return A functor which complies with the \ref revkit::truth_table_synthesis_func "truth_table_synthesis_func" interface
   *
   * @since  1.0
   */
  truth_table_synthesis_func swop_func( properties::ptr settings = std::make_shared<properties>(), properties::ptr statistics = std::make_shared<properties>() );

}

#endif /* SWOP_HPP */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
