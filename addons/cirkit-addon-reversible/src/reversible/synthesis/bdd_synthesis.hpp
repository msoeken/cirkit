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
 * @file bdd_synthesis.hpp
 *
 * @brief BDD interface for DD-based synthesis
 *
 * @author Mathias Soeken
 * @since  1.0
 */

#ifndef BDD_SYNTHESIS_HPP
#define BDD_SYNTHESIS_HPP

#include <string>

#include <core/properties.hpp>

#include <reversible/circuit.hpp>
#include <reversible/synthesis/synthesis.hpp>

namespace cirkit
{

  /**
   * @brief BDD based synthesis approach
   *
   * This algorithm implements the BDD based synthesis approach based on [\ref WD09].
   * It supports complemented edges, different re-ordering strategies and the generation
   * of both, Toffoli and elementary quantum gates.
   *
   * The function representation can be read from a PLA file-name.
   *
   * @param circ The circuit to be constructed
   * @param filename A PLA file
   * @param settings <table border="0" width="100%">
   *   <tr>
   *     <td class="indexkey">Setting</td>
   *     <td class="indexkey">Type</td>
   *     <td class="indexkey">Default Value</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">complemented_edges</td>
   *     <td class="indexvalue">bool</td>
   *     <td class="indexvalue">true</td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">Specifies whether complemented edges should be used for the BDD.</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">reordering</td>
   *     <td class="indexvalue">unsigned</td>
   *     <td class="indexvalue">CUDD_REORDER_SIFT</td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">Reordering of the BDD. Check <a href="http://vlsi.colorado.edu/~fabio/CUDD/node3.html#SECTION000312000000000000000">CUDD Manual</a> for details.</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">dotfilename</td>
   *     <td class="indexvalue">std::string</td>
   *     <td class="indexvalue">std::string()</td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">If not empty a DOT representation of the BDD is dumped to the file-name.</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">infofilename</td>
   *     <td class="indexvalue">std::string</td>
   *     <td class="indexvalue">std::string()</td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">If not empty information about the BDD is dumped to the file-name.</td>
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
   *     <td class="indexvalue">Number of nodes of the BDD.</td>
   *   </tr>
   * </table>
   * @return true on success
   *
   * @since  1.0
   */
  bool bdd_synthesis( circuit& circ, const std::string& filename,
                      properties::ptr settings = properties::ptr(),
                      properties::ptr statistics = properties::ptr() );

  /**
   * @brief Functor for the \ref revkit::bdd_synthesis_func "bdd_synthesis_func" algorithm
   *
   * @param settings Settings (see \ref revkit::bdd_synthesis_func "bdd_synthesis_func")
   * @param statistics Statistics (see \ref revkit::bdd_synthesis_func "bdd_synthesis_func")
   *
   * @return A functor which complies with the \ref revkit::pla_blif_synthesis_func "pla_blif_synthesis_func" interface
   *
   * @since  1.0
   */
  pla_blif_synthesis_func bdd_synthesis_func( properties::ptr settings = std::make_shared<properties>(), properties::ptr statistics = std::make_shared<properties>() );

}

#endif /* BDD_SYNTHESIS_HPP */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
