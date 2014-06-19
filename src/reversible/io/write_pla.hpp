/* RevKit: A Toolkit for Reversible Circuit Design (www.revkit.org)
 * Copyright (C) 2009-2014  The RevKit Developers <revkit@informatik.uni-bremen.de>
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
 * @file write_pla.hpp
 *
 * @brief Write PLA truth table to file
 *
 * @author Mathias Soeken
 * @since  2.0
 */

#ifndef WRITE_PLA_HPP
#define WRITE_PLA_HPP

#include <string>

#include <reversible/truth_table.hpp>

namespace cirkit
{

  /**
   * @brief Writes PLA truth table to file
   *
   * @param pla PLA given as truth table (can contain don't cares)
   * @param filename PLA filename
   *
   * @version 2.0
   */
  void write_pla( const binary_truth_table& pla, const std::string& filename );

}

#endif

// Local Variables:
// c-basic-offset: 2
// End:
