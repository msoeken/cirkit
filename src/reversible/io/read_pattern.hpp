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
 * @file read_pattern.hpp
 *
 * @brief Parser for Simulation pattern
 *
 * @author Mathias Soeken
 * @since  1.2
 */

#ifndef READ_PATTERN_HPP
#define READ_PATTERN_HPP

#include <string>

namespace cirkit
{

  class pattern;

  /**
   * @brief I/O routine for reading a pattern (*.sim) file
   *
   * @param p Empty pattern class
   * @param filename File-name of the pattern file
   * @param error If not null, the string targeted by the pointer is assigned with
   *              an error message
   *
   * @since  1.2
   */
  bool read_pattern( pattern& p, const std::string& filename, std::string* error = 0 );

}

#endif /* READ_PATTERN_HPP */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
