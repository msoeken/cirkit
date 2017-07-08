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
 * @file embed_truth_table.hpp
 *
 * @brief Embedding of an irreversible specification
 *
 * @author Mathias Soeken
 * @since  1.0
 */

#ifndef EMBED_TRUTH_TABLE_HPP
#define EMBED_TRUTH_TABLE_HPP

#include <classical/utils/truth_table_utils.hpp>
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
                        const properties::ptr& settings = properties::ptr(),
                        const properties::ptr& statistics = properties::ptr() );

bool embed_truth_table( binary_truth_table& spec, const tt& base,
                        const properties::ptr& settings = properties::ptr(),
                        const properties::ptr& statistics = properties::ptr() );

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
embedding_func embed_truth_table_func( const properties::ptr& settings = std::make_shared<properties>(),
                                       const properties::ptr& statistics = std::make_shared<properties>() );

}

#endif /* EMBED_TRUTH_TABLE_HPP */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
