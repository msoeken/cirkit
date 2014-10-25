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
 * @file transposition_to_circuit.hpp
 *
 * @brief Creates a circuit realization for a transposition
 *
 * @author Mathias Soeken
 * @author Laura Tague
 * @since  1.3
 */

#ifndef PATTERN_TO_CIRCUIT_HPP
#define PATTERN_TO_CIRCUIT_HPP

#include <boost/dynamic_bitset.hpp>

namespace cirkit
{
  class circuit;

/**
 * @brief Creates a circuit realization for a permutation of two patterns
 *
 * This function takes two lines from a reversible truth table (inputs -> pattern1)
 * (inputs -> pattern2) which need to be swapped,
 * and returns a circuit that will swap the two patterns assigment
 * and vice versa by leaving all other input and output assignments unaltered.
 *
 * @param circ An empty circuit that will be created. The number of lines of the circuit
 *             must coincide with the length of the input and output assignments.
 * @param inputs pattern1 Assignment
 * @param outputs pattern2 Assignment
 *
 * @since 2.0
 */
void pattern_to_circuit( circuit& circ, const boost::dynamic_bitset<>& pattern1, const boost::dynamic_bitset<>& pattern2);

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
