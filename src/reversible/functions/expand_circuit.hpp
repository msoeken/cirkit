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
 * @file expand_circuit.hpp
 *
 * @brief Expand a circuit on the base of a sub circuit
 *
 * @author Mathias Soeken
 * @since  1.0
 */

#ifndef EXPAND_CIRCUIT_HPP
#define EXPAND_CIRCUIT_HPP

#include <vector>

#include <reversible/circuit.hpp>

namespace revkit
{

  /**
   * @brief Expands a circuit
   *
   * This function expands the circuit \p base, such that
   * it will have \p num_lines and maps each line \em i
   * in the circuit \p base to the line \em filter[i] in the
   * circuit \p circ.
   *
   * @param base Base circuit
   * @param circ Newly created circuit, extended from \p base. Needs to be empty.
   * @param num_lines New number of lines
   * @param filter Mapping for calculating the new line indices.
   *
   * @return true on success, false otherwise
   *
   * @since  1.0
   */
  bool expand_circuit( const circuit& base, circuit& circ, unsigned num_lines, const std::vector<unsigned>& filter );

}

#endif /* EXPAND_CIRCUIT_HPP */

// Local Variables:
// c-basic-offset: 2
// End:
