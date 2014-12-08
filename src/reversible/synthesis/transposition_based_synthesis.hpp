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
 * @file transposition_based_synthesis.hpp
 *
 * @brief A simple synthesis algorithm based on transpositions
 *
 * @author Mathias Soeken
 * @since  1.3
 */
#ifndef TRANSPOSITION_BASED_SYNTHESIS
#define TRANSPOSITION_BASED_SYNTHESIS

#include <core/properties.hpp>

#include <reversible/circuit.hpp>
#include <reversible/truth_table.hpp>
#include <boost/dynamic_bitset.hpp>
#include <reversible/synthesis/synthesis.hpp>

namespace cirkit
{

  /**
   * @brief A simple synthesis algorithm based on transpositions
   *
   * @since  1.3
   */
  bool transposition_based_synthesis( circuit& circ, const binary_truth_table& spec, properties::ptr settings = properties::ptr(), properties::ptr statistics = properties::ptr() );

  /**
   * @brief Functor for the transposition_based_synthesis algorithm
   *
   * @param settings Settings (see transposition_based_synthesis)
   * @param statistics Statistics (see transposition_based_synthesis)
   *
   * @return A functor which complies with the truth_table_based_synthesis_func interface
   *
   * @since  1.3
   */
  truth_table_synthesis_func transposition_based_synthesis_func( properties::ptr settings = std::make_shared<properties>(), properties::ptr statistics = std::make_shared<properties>() );

}

#endif /* TRANSPOSITION_BASED_SYNTHESIS */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
