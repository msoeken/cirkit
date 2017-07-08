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
 * @file transformation_based_synthesis.hpp
 *
 * @brief Transformation Based Synthesis
 *
 * @author Mathias Soeken
 * @since  1.0
 */

#ifndef TRANSFORMATION_BASED_SYNTHESIS
#define TRANSFORMATION_BASED_SYNTHESIS

#include <core/properties.hpp>
#include <reversible/circuit.hpp>
#include <reversible/truth_table.hpp>

#include <reversible/synthesis/synthesis.hpp>

namespace cirkit
{

/**
 * @brief Synthesizes a circuit using the Transformation Based approach
 *
 * This function implements the algorithm published in [\ref MMD03].
 *
 * @param circ       Empty Circuit
 * @param spec       Function Specification (has to be fully specified)
 * @param settings <table border="0" width="100%">
 *   <tr>
 *     <td class="indexkey">Setting</td>
 *     <td class="indexkey">Type</td>
 *     <td class="indexkey">Default Value</td>
 *   </tr>
 *   <tr>
 *     <td rowspan="2" class="indexvalue">bidirectional</td>
 *     <td class="indexvalue">bool</td>
 *     <td class="indexvalue">true</td>
 *   </tr>
 *   <tr>
 *     <td colspan="2" class="indexvalue">Use the bidirectional approach as described in [\ref MMD03].</td>
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
 *
 * @return true if successful, false otherwise
 *
 * @since  1.0
 */
bool transformation_based_synthesis( circuit& circ, const binary_truth_table& spec,
                                     const properties::ptr& settings = properties::ptr(),
                                     const properties::ptr& statistics = properties::ptr() );

/**
 * @brief Functor for the \ref revkit::transformation_based_synthesis "transformation_based_synthesis" algorithm
 *
 * @param settings Settings (see \ref revkit::transformation_based_synthesis "transformation_based_synthesis")
 * @param statistics Statistics (see \ref revkit::transformation_based_synthesis "transformation_based_synthesis")
 *
 * @return A functor which complies with the \ref revkit::truth_table_synthesis_func "truth_table_based_synthesis_func" interface
 *
 * @since  1.0
 */
truth_table_synthesis_func transformation_based_synthesis_func( const properties::ptr& settings = std::make_shared<properties>(),
                                                                const properties::ptr& statistics = std::make_shared<properties>() );

}

#endif /* TRANSFORMATION_BASED_SYNTHESIS */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
