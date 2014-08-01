/* RevKit (www.rekit.org)
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
 * @file approximate_additional_lines.hpp
 *
 * @brief Approximates the number of additional lines
 *
 * @author Mathias Soeken
 * @since  2.0
 */

#ifndef APPROXIMATE_ADDITIONAL_LINES_HPP
#define APPROXIMATE_ADDITIONAL_LINES_HPP

#include <string>

#include <core/properties.hpp>

namespace cirkit
{

  /**
   * @brief Approximates the number of additional lines
   *
   * @param filename Filename to a PLA representation
   * @param statistics If statistics are set the original number of inputs and outputs of the
   *                   function are stored in the fields "num_inputs" and "num_outputs"
   *
   * @return Number of additional lines that are sufficient for embedding
   *
   * @version 2.0
   */
  unsigned approximate_additional_lines( const std::string& filename,
                                         properties::ptr settings = properties::ptr(),
                                         properties::ptr statistics = properties::ptr() );

}

#endif

// Local Variables:
// c-basic-offset: 2
// End:
