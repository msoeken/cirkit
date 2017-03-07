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
 * @file line_reduction.hpp
 *
 * @brief Line Reduction Optimization
 *
 * @author Mathias Soeken
 * @since  1.0
 */

#ifndef LINE_REDUCTION_HPP
#define LINE_REDUCTION_HPP

#include <iostream>

#include <core/properties.hpp>
#include <reversible/truth_table.hpp>

#include <reversible/optimization/optimization.hpp>
#include <reversible/synthesis/synthesis.hpp>

namespace cirkit
{

  class circuit;

  /**
   * @brief Functor for re-synthesis in the revkit::line_reduction algorithm
   *
   * This functor needs in addition to the truth_table_synthesis_func functor an output order.
   * This output order can be passed to the embedding algorithm.
   *
   * @since  1.0
   */
  typedef boost::function<bool(circuit&, binary_truth_table&, const std::vector<unsigned>& order)> window_synthesis_func;

  /**
   * @brief Concrete re-synthesis functor for the revkit::line_reduction algorithm
   *
   * This functor calls an embedding algorithm to embed the specification using the output order.
   * Afterwards, a synthesis algorithm is called with the modified specification.
   * Both, embedding and synthesis algorithm, can be set using parameters.
   *
   * @since  1.0
   */
  struct embed_and_synthesize
  {
    /**
     * @brief Standard constructor
     *
     * Initializes default values
     *
     * @since  1.0
     */
    embed_and_synthesize();

    /**
     * @brief Embedding algorithm to use
     *
     * Default value is \b embed_truth_table_func
     *
     * @since  1.0
     */
    embedding_func embedding;

    /**
     * @brief Synthesis algorithm to use
     *
     * Default value is \b transformation_based_synthesis_func
     *
     * @since  1.0
     */
    truth_table_synthesis_func synthesis;

    /**
     * @brief Time-out for the synthesis algorithm to use
     *
     * The time is given in milliseconds. If time is 0u,
     * no timeout is used. The default value is 0u.
     *
     * @since  1.1
     */
    unsigned timeout;

    /**
     * @brief Functor operator implementation
     *
     * Calls the embedding algorithm with the output order and afterwards the synthesis algorithm with the circuit.
     *
     * Both, circuit and specification, are modified by this operator.
     *
     * The circuit has to be empty.
     *
     * @since  1.0
     */
    bool operator()( circuit& circ, binary_truth_table& spec, const std::vector<unsigned>& output_order );
  };

  /**
   * @brief Line Reduction Optimization
   *
   * This algorithm implements the approach presented in [\ref WSD10].
   * Windows are found and re-synthesized such that an output of that window is always returning a constant value,
   * so that it can be used as replacement for another constant input line, often introduced by hierarchical synthesis
   * methods.
   *
   * @param circ Optimized circuit to be created (needs to be empty)
   * @param base Base circuit which should be optimized
   * @param settings <table border="0" width="100%">
   *   <tr>
   *     <td class="indexkey">Setting</td>
   *     <td class="indexkey">Type</td>
   *     <td class="indexkey">Default Value</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">max_window_lines</td>
   *     <td class="indexvalue">unsigned</td>
   *     <td class="indexvalue">6u</td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">Number of lines the selected windows can have initially.</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">max_grow_up_window_lines</td>
   *     <td class="indexvalue">unsigned</td>
   *     <td class="indexvalue">9u</td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">When the truth table is not reversible, obtained by a window with \em max_window_lines lines, then the number of lines can be increased up at most this value.</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">window_variables_threshold</td>
   *     <td class="indexvalue">unsigned</td>
   *     <td class="indexvalue">17u</td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">The possible window inputs are obtained by simulating its \em cone \em of \em influence. It is only simulated if the number of its primary inputs is less or equal to this value.</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">simulation</td>
   *     <td class="indexvalue">\ref revkit::simulation_func "simulation_func"</td>
   *     <td class="indexvalue">\ref revkit::simple_simulation_func "simple_simulation_func()"</td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">Simulation function used to simulate values inside the windows and inside the \em cone \em of \em influence.</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">window_synthesis</td>
   *     <td class="indexvalue">\ref revkit::window_synthesis_func "window_synthesis_func"</td>
   *     <td class="indexvalue">\ref revkit::embed_and_synthesize "embed_and_synthesize()"</td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">Functor used to re-synthesize the window. It only has to embed and synthesize the window. It is preferred to use \ref revkit::embed_and_synthesize "embed_and_synthesize", whereby the parameters can be adjusted to use different synthesis algorithms.</td>
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
   *   <tr>
   *     <td class="indexvalue">num_considered_windows</td>
   *     <td class="indexvalue">unsigned</td>
   *     <td class="indexvalue">Number of windows, which were considered in total.</td>
   *   </tr>
   *   <tr>
   *     <td class="indexvalue">skipped_max_window_lines</td>
   *     <td class="indexvalue">unsigned</td>
   *     <td class="indexvalue">Number of skipped windows due to maximum number of allowed primary inputs to be simulated, see \em window_variables_threshold.</td>
   *   </tr>
   *   <tr>
   *     <td class="indexvalue">skipped_ambiguous_line</td>
   *     <td class="indexvalue">unsigned</td>
   *     <td class="indexvalue">Number of skipped windows due to irreversible specification.</td>
   *   </tr>
   *   <tr>
   *     <td class="indexvalue">skipped_no_constant_line</td>
   *     <td class="indexvalue">unsigned</td>
   *     <td class="indexvalue">Number of skipped windows in the case that no constant line can be found for a garbage line.</td>
   *   </tr>
   *   <tr>
   *     <td class="indexvalue">skipped_synthesis_failed</td>
   *     <td class="indexvalue">unsigned</td>
   *     <td class="indexvalue">Number of skipped windows in the case that the synthesis of the window failed.</td>
   *   </tr>
   * </table>
   * @return true on success
   *
   * @since  1.0
   */
  bool line_reduction( circuit& circ, const circuit& base, properties::ptr settings = properties::ptr(), properties::ptr statistics = properties::ptr() );

  /**
   * @brief Functor for the \ref revkit::line_reduction "line_reduction" algorithm
   *
   * @param settings Settings (see \ref revkit::line_reduction "line_reduction")
   * @param statistics Statistics (see \ref revkit::line_reduction "line_reduction")
   *
   * @return A functor which complies with the \ref revkit::optimization_func "optimization_func" interface
   *
   * @since  1.0
   */
  optimization_func line_reduction_func(  properties::ptr settings = std::make_shared<properties>(), properties::ptr statistics = std::make_shared<properties>() );
}

#endif /* LINE_REDUCTION_HPP */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
