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
 * @file kfdd_synthesis.hpp
 *
 * @brief KFDD interface for DD-based synthesis
 *
 * @author Mathias Soeken
 * @since  1.0
 */

#ifndef KFDD_SYNTHESIS_HPP
#define KFDD_SYNTHESIS_HPP

#include <string>

#include <core/properties.hpp>
#include <reversible/circuit.hpp>

#include <reversible/synthesis/synthesis.hpp>

namespace cirkit
{

  // NOTE define naming conventions for public enumerations
  /**
   * @brief Flags for default KFDD decomposition in kfdd_synthesis_settings
   *
   * The default decomposition type is especially important when no
   * DTL reordering strategy is used.
   *
   * @since  1.0
   */
  enum {
    /**
     * @brief Use Shannon as default
     */
    kfdd_synthesis_dtl_shannon,
    /**
     * @brief Use positive Davio as default
     */
    kfdd_synthesis_dtl_positive_davio,
    /**
     * @brief Use negative Davio as default
     */
    kfdd_synthesis_dtl_negative_davio };

  /**
   * @brief Flags for KFDD Reordering strategies in kfdd_synthesis_settings
   *
   * @since  1.0
   */
  enum {
    /**
     * @brief No reordering
     */
    kfdd_synthesis_reordering_none,
    /**
     * @brief Exact DTL and variable re-ordering according to an algorithm introduced by Friedman
     */
    kfdd_synthesis_reordering_exact_dtl_friedman,
    /**
     * @brief Exact DTL and variable re-ordering by permutation
     */
    kfdd_synthesis_reordering_exact_dtl_permutation,
    /**
     * @brief Heuristic DTL and variable re-ordering by sifting
     */
    kfdd_synthesis_reordering_dtl_sifting,
    /**
     * @brief Exact variable re-ordering according to an algorithm introduced by Friedman
     */
    kfdd_synthesis_reordering_exact_friedman,
    /**
     * @brief Exact variable re-ordering by permutation
     */
    kfdd_synthesis_reordering_exact_permutation,
    /**
     * @brief Heuristic variable re-ordering by sifting
     */
    kfdd_synthesis_reordering_sifting,
    /**
     * @brief Heuristic variable re-ordering by sifting followed by heuristic DTL and variable re-ordering by sifting
     */
    kfdd_synthesis_reordering_sifting_and_dtl_sifting,
    /**
     * @brief Inversion of the variable ordering
     */
    kfdd_synthesis_reordering_inverse };

  /**
   * @brief Flags for the growth limit in kfdd_synthesis_settings
   */
  enum {
    /**
     * @brief Relative growth limit: after each repositioning of a sifting variable the comparison size for the growing is actualized
     */
    kfdd_synthesis_growth_limit_relative = 'r',
    /**
     * @brief Absolute growth limit: the comparison size is the intial size of the DDs for the complete sifting-process
     */
    kfdd_synthesis_growth_limit_absolute = 'a' };

  /**
   * @brief Flags for the sifting method
   */
  enum {
    /**
     * @brief \b Random
     */
    kfdd_synthesis_sifting_method_random = 'r',
    /**
     * @brief \b Initial: The sifting variables are chosen in the order given before the sifting procedure starts
     */
    kfdd_synthesis_sifting_method_initial = 'i',
    /**
     * @brief \b Greatest: Chooses the variable in the level with the largest number of nodes
     */
    kfdd_synthesis_sifting_method_greatest = 'g',
    /**
     * @brief <b>Loser first:</b> Although the complete sifting process will reduce the number of DD-Nodes (or at least keep the same size if no improvement can be done) after each repositioning of a sifting variable there will occasionally be some levels that grow. The loser first strategy chooses the next sifting candidate as the variable in the level with the least increase in size
     */
    kfdd_synthesis_sifting_method_loser_first = 'l',
    /**
     * @brief \b Verify: Calculates the number of node eliminations due to the reduction rules of KFDDs if a variable is repositioned in a specific level. It then chooses the best position according to the highest count result
     */
    kfdd_synthesis_sifting_method_verify = 'v' };

  /**
   * @brief KFDD Based Synthesis
   *
   * This algorithm implements the KFDD based synthesis approach as introduced in [\ref SWD10]. Thereby, re-ordering strategies as well as different decomposition types can be used.
   *
   * @param circ Empty circuit, to be constructed by the algorithm
   * @param filename A functional representation as BLIF or PLA file
   * @param settings <table border="0" width="100%">
   *   <tr>
   *     <td class="indexkey">Setting</td>
   *     <td class="indexkey">Type</td>
   *     <td class="indexkey">Default Value</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">default_decomposition</td>
   *     <td class="indexvalue">unsigned</td>
   *     <td class="indexvalue">revkit::kfdd_synthesis_dtl_shannon</td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">Default decomposition type when initially generating the KFDD.</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">reordering</td>
   *     <td class="indexvalue">unsigned</td>
   *     <td class="indexvalue">revkit::kfdd_synthesis_reordering_none</td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">Reordering of the KFDD.</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">sift_factor</td>
   *     <td class="indexvalue">double</td>
   *     <td class="indexvalue">2.5</td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">Sifting factor, used when reordering uses sifting.</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">sifting_growth_limit</td>
   *     <td class="indexvalue">unsigned</td>
   *     <td class="indexvalue">revkit::kfdd_synthesis_growth_limit_absolute</td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">Sifting growth limit, used when reordering uses sifting.</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">sifting_method</td>
   *     <td class="indexvalue">char</td>
   *     <td class="indexvalue">revkit::kfdd_synthesis_sifting_method_verify</td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">Sifting method, used when reordering uses sifting.</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">dotfilename</td>
   *     <td class="indexvalue">std::string</td>
   *     <td class="indexvalue">std::string()</td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">If not empty a DOT representation of the KFDD is dumped to the file-name.</td>
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
   *     <td class="indexvalue">node_count</td>
   *     <td class="indexvalue">unsigned</td>
   *     <td class="indexvalue">Number of nodes of the KFDD.</td>
   *   </tr>
   * </table>
   * @return true on success
   *
   * @since  1.0
   */
  bool kfdd_synthesis( circuit& circ, const std::string& filename,
                       properties::ptr settings = properties::ptr(),
                       properties::ptr statistics = properties::ptr() );

  /**
   * @brief Functor for the \ref revkit::kfdd_synthesis_func "kfdd_synthesis_func" algorithm
   *
   * @param settings Settings (see \ref revkit::kfdd_synthesis_func "kfdd_synthesis_func")
   * @param statistics Statistics (see \ref revkit::kfdd_synthesis_func "kfdd_synthesis_func")
   *
   * @return A functor which complies with the \ref revkit::pla_blif_synthesis_func "pla_blif_synthesis_func" interface
   *
   * @since  1.0
   */
  pla_blif_synthesis_func kfdd_synthesis_func( properties::ptr settings = std::make_shared<properties>(), properties::ptr statistics = std::make_shared<properties>() );

}

#endif /* KFDD_SYNTHESIS_HPP */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
