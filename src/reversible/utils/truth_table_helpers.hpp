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
 * @file truth_table_helpers.hpp
 *
 * @brief Some useful functions for dealing with truth tables
 *
 * @author Mathias Soeken
 * @since  1.0
 */

#ifndef TRUTH_TABLE_HELPERS_HPP
#define TRUTH_TABLE_HELPERS_HPP

#include <vector>

#include <boost/dynamic_bitset.hpp>

#include <reversible/truth_table.hpp>

namespace cirkit
{

typedef std::vector<boost::dynamic_bitset<> > bitset_vector_t;

/**
 * Transforms a truth table into a vector of bitsets
 *
 * @param spec A fully specified truth table for a total reversible function
 * @param vec  An empty vector
 */
void truth_table_to_bitset_vector( const binary_truth_table& spec, bitset_vector_t& vec );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
