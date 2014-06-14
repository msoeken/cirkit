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
 * @file embed_truth_table.hpp
 *
 * @brief Embedding of an irreversible specification
 *
 * @author Mathias Soeken
 * @since  1.0
 */

#ifndef EMBED_TRUTH_TABLE_HPP
#define EMBED_TRUTH_TABLE_HPP

#include <reversible/truth_table.hpp>

#include <reversible/synthesis/synthesis.hpp>

namespace cirkit
{

  /**
   * @brief Embedding of an irreversible specification
   *
   * This algorithm takes an irreversible (incompletely) specified truth table, for example
   * using \ref revkit::read_pla "read_pla" and embeds it into a reversible specification.
   * Thereby necessary garbage and constant lines are added.
   * The function is always embedded using the 0 values of the constant lines
   * and the method which is used is the "Greedy Method" applying possible assignments
   * by the minimal hamming distance.
   *
   * @section sec_embed_truth_table_example Example
   * @code
   * binary_truth_table base, spec;
   * read_pla( base, "filename.pla" );
   * embed_truth_table( spec, base );
   * @endcode
   *
   * @param spec Embedded specification (it can be the same as base to embed in place)
   * @param base Specification to be embedded
   * @param settings <table border="0" width="100%">
   *   <tr>
   *     <td class="indexkey">Setting</td>
   *     <td class="indexkey">Type</td>
   *     <td class="indexkey">Default Value</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">default_decomposition</td>
   *     <td class="indexvalue">garbage_name</td>
   *     <td class="indexvalue">"g"</td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">Name of the output of the helper line, if added by the algorithm.</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">output_order</td>
   *     <td class="indexvalue">std::vector&lt;unsigned&gt;</td>
   *     <td class="indexvalue"><i>empty vector</i></td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">Order of the output when embedding. The vector contains indices of all possible positions. A valid unique position for each output has to be specified. If empty the first lines are used for the outputs in order.</td>
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
  bool embed_truth_table( binary_truth_table& spec, const binary_truth_table& base,
                          properties::ptr settings = properties::ptr(),
                          properties::ptr statistics = properties::ptr() );

  /**
   * @brief Functor for the \ref revkit::embed_truth_table "embed_truth_table" algorithm
   *
   * @param settings Settings (see \ref revkit::embed_truth_table "embed_truth_table")
   * @param statistics Statistics (see \ref revkit::embed_truth_table "embed_truth_table")
   *
   * @return A functor which complies with the \ref revkit::embedding_func "embedding_func" interface
   *
   * @since  1.0
   */
  embedding_func embed_truth_table_func( properties::ptr settings = properties::ptr( new properties() ), properties::ptr statistics = properties::ptr( new properties() ) );

}

#endif /* EMBED_TRUTH_TABLE_HPP */

// Local Variables:
// c-basic-offset: 2
// End:
