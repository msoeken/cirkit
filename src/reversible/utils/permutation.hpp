/* CirKit: A circuit toolkit
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
 * @file permutation.hpp
 *
 * @brief Permutation helpers
 *
 * @author Mathias Soeken
 * @since  2.1
 */

#ifndef PERMUTATION_HPP
#define PERMUTATION_HPP

#include <vector>

#include <reversible/circuit.hpp>
#include <reversible/truth_table.hpp>

namespace cirkit
{

using permutation_t = std::vector<unsigned>;
using cycles_t      = std::vector<std::vector<unsigned>>;

permutation_t truth_table_to_permutation( const binary_truth_table& spec );
permutation_t circuit_to_permutation( const circuit& circ );
cycles_t permutation_to_cycles( const permutation_t& perm, bool sort = true );
unsigned permutation_inv( const permutation_t& perm );
int permutation_sign( const permutation_t& perm );
std::vector<unsigned> cycles_type( const cycles_t& cycles );
bool is_involution( const permutation_t& perm );

std::string permutation_to_string( const permutation_t& perm );
std::string cycles_to_string( const cycles_t& cycles, bool print_fixpoints = false );
std::string cycles_to_string( const permutation_t& perm, bool print_fixpoints = false );
std::string type_to_string( const std::vector<unsigned>& type );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
