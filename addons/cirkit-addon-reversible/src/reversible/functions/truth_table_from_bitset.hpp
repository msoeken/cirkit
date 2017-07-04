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
 * @file truth_table_from_bitset.hpp
 *
 * @brief Generates a truth table from a bitset
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef TRUTH_TABLE_FROM_BITSET_HPP
#define TRUTH_TABLE_FROM_BITSET_HPP

#include <boost/dynamic_bitset.hpp>

#include <reversible/truth_table.hpp>

namespace cirkit
{

/* e.g.: maps 0001 into
 *
 *  00 0
 *  01 0
 *  10 0
 *  11 1
 */
binary_truth_table truth_table_from_bitset_direct( const boost::dynamic_bitset<>& bs );


/* e.g.: maps 0001 into
 *
 *  000 0--
 *  001 0--
 *  010 0--
 *  011 1--
 *  100 ---
 *  101 ---
 *  110 ---
 *  111 ---
 *
 * requires one fewer line if bs is balanced
 */
binary_truth_table truth_table_from_bitset( const boost::dynamic_bitset<>& bs );

/* e.g.: maps 0001 into
 *
 *   00 0  00 0
 *   00 1  00 1
 *   01 0  01 0
 *   01 1  01 1
 *   10 0  10 0
 *   10 1  10 1
 *   11 0  11 1
 *   11 1  11 0
 */
binary_truth_table truth_table_from_bitset_bennett( const boost::dynamic_bitset<>& bs );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
