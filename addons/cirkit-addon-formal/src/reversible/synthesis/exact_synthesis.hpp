/* RevKit (www.revkit.org)
 * Copyright (C) 2009-2014  University of Bremen
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
 * @file exact_synthesis.hpp
 *
 * @brief Exact Synthesis of Reversible Networks
 *
 * @author Mathias Soeken
 * @author Oliver Keszöcze
 * @author Stefan Frehse
 * @since  1.0
 */

#if ADDON_FORMAL

#ifndef EXACT_SYNTHESIS_HPP
#define EXACT_SYNTHESIS_HPP

#include <core/properties.hpp>

#include <reversible/circuit.hpp>
#include <reversible/truth_table.hpp>
#include <reversible/synthesis/synthesis.hpp>

namespace cirkit
{

  /**
   * @brief Synthesizes a minimal circuit (with respect to the number of gates) using the SAT-based exact synthesis approach as presented in [\ref GWDD09].
   *
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
   *     <td rowspan="2" class="indexvalue">max_depth</td>
   *     <td class="indexvalue">unsigned</td>
   *     <td class="indexvalue">20u</td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">The maximal considered circuit depth.</td>
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
   * @author RevKit
   * @since  1.0
   */
  bool exact_synthesis( circuit& circ
       , const binary_truth_table& spec
       , properties::ptr settings = properties::ptr ()
       , properties::ptr statistics = properties::ptr () );

  /**
   * @brief Functor for the \ref revkit::exact_synthesis "exact_synthesis" algorithm
   *
   * @param settings Settings (see \ref revkit::exact_synthesis "exact_synthesis")
   * @param statistics Statistics (see \ref revkit::exact_synthesis "exact_synthesis")
   *
   * @return A functor which complies with the \ref revkit::exact_synthesis "exact_synthesis_func" interface
   *
   * @author RevKit
   * @since  1.0
   */
  truth_table_synthesis_func exact_synthesis_func( properties::ptr settings = std::make_shared<properties>(),
                                                   properties::ptr statistics = std::make_shared<properties>() );

}

#endif /* EXACT_SYNTHESIS_HPP */

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
