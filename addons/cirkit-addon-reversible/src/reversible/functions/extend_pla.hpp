/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2016  EPFL
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
 * @file extend_pla.hpp
 *
 * @brief Extends a PLA
 *
 * @author Mathias Soeken
 * @since  2.0
 */

#include <reversible/truth_table.hpp>

#ifndef EXTEND_PLA_HPP
#define EXTEND_PLA_HPP

namespace cirkit
{

  /**
   * @brief Settings for extend_pla_settings
   *
   * @since  2.0
   */
  struct extend_pla_settings
  {
    /**
     * @brief Compact extended PLA after creation
     *
     * If this settings is set to true (which is default)
     * the extended PLA is compacted in a post process.
     * Although, the PLA becomes significantly smaller,
     * it may still be exponential in the worst case before this
     * step.
     *
     * @since  2.0
     */
    bool post_compact = true;

    /**
     * @brief Be verbose
     *
     * @since  2.0
     */
    bool verbose = false;
  };

  /**
   * @brief Extends a PLA representation such that it does
   *        not contain any intersecting input cubes.
   *
   * Caution: Note that this function changes the input parameter base.
   *
   * @param base     The original PLA representation
   * @param extended The extended new PLA representation
   * @param settings Settings
   *
   * @since  2.0
   */
  void extend_pla( binary_truth_table& base, binary_truth_table& extended, const extend_pla_settings& settings = extend_pla_settings() );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
