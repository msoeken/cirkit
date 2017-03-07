/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2017  EPFL
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
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

using bitset_vector_t      = std::vector<boost::dynamic_bitset<> >;
using bitset_pair_vector_t = std::vector<std::pair<boost::dynamic_bitset<>, boost::dynamic_bitset<>>>;

/**
 * Transforms a truth table into a vector of bitsets
 *
 * @param spec A fully specified truth table for a total reversible function
 */
bitset_vector_t truth_table_to_bitset_vector( const binary_truth_table& spec );

bitset_pair_vector_t truth_table_to_bitset_pair_vector( const binary_truth_table& spec );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
