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
#include <core/truth_table.hpp>

#include <algorithms/optimization/optimization.hpp>
#include <algorithms/synthesis/synthesis.hpp>

namespace revkit
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
  optimization_func line_reduction_func( properties::ptr settings = properties::ptr( new properties() ), properties::ptr statistics = properties::ptr( new properties() ) );
}

#endif /* LINE_REDUCTION_HPP */

// Local Variables:
// c-basic-offset: 2
// End:
