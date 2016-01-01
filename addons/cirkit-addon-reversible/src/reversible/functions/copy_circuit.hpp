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
 * @file copy_circuit.hpp
 *
 * @brief Copies a circuit
 *
 * @author Mathias Soeken
 * @since  1.0
 */

#ifndef COPY_CIRCUIT_HPP
#define COPY_CIRCUIT_HPP

#include <vector>

#include <reversible/circuit.hpp>

namespace cirkit
{

  /**
   * @brief Copies a circuit with all meta information
   *
   * This function creates a copy of the circuit \p src in \p dest
   * including all meta information as input and output names,
   * and also constant input and garbage output information.
   *
   * @param src  Source circuit
   * @param dest Destination circuit
   *
   * @since  1.0
   */
  void copy_circuit( const circuit& src, circuit& dest );

  /**
   * @brief Copies a circuit based on a line filter
   *
   * This function creates a copy of the circuit \p src in \p dest
   * but considers only the lines that have been specified in \p filter.
   * The caller of the function is reponsible that \p dest still contains
   * valid gates, e.g. by checking that all lines that are not in filter
   * are empty or only contain control lines.
   *
   * @param src  Source circuit
   * @param dest Destination circuit
   *
   * @since  2.0
   */
  void copy_circuit( const circuit& src, circuit& dest, const std::vector<unsigned>& filter );


}

#endif /* COPY_CIRCUIT_HPP */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
