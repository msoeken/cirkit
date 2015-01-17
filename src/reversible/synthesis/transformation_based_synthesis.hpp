/* RevKit (www.revkit.org)
 * Copyright (C) 2009-2015  University of Bremen
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
                                       properties::ptr settings = properties::ptr(),
                                       properties::ptr statistics = properties::ptr() );

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
  truth_table_synthesis_func transformation_based_synthesis_func( properties::ptr settings = std::make_shared<properties>(), properties::ptr statistics = std::make_shared<properties>() );

}

#endif /* TRANSFORMATION_BASED_SYNTHESIS */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
