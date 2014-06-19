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
 * @file reverse_circuit.hpp
 *
 * @brief Reverse a circuit
 *
 * @author Mathias Soeken
 * @since  1.0
 */

#ifndef REVERSE_CIRCUIT_HPP
#define REVERSE_CIRCUIT_HPP

#include <reversible/circuit.hpp>

namespace cirkit
{

  /**
   * @brief Reverse a circuit
   *
   * This function reverses a circuit \p src and writes
   * the result to a new circuit \p dest.
   *
   * You can use reverse_circuit(circuit&) if the circuit
   * should be reversed in-place.
   *
   * @param src  Source circuit
   * @param dest Destination circuit
   *
   * @since  1.0
   */
  void reverse_circuit( const circuit& src, circuit& dest );

  /**
   * @brief Reverse a circuit in-place
   *
   * This function reverses a circuit \p circ in-place.
   *
   * @param circ Circuit to be reversed
   *
   * @since  1.0
   */
  void reverse_circuit( circuit& circ );

}

#endif /* REVERSE_CIRCUIT_HPP */

// Local Variables:
// c-basic-offset: 2
// End:
