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
 * @file xmg_utils.hpp
 *
 * @brief XMG utility functions
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef XMG_UTILS_HPP
#define XMG_UTILS_HPP

#include <deque>
#include <iostream>
#include <stack>
#include <vector>

#include <boost/dynamic_bitset.hpp>

#include <classical/utils/truth_table_utils.hpp>
#include <classical/xmg/xmg.hpp>

namespace cirkit
{

unsigned compute_depth( const xmg_graph& xmg );

void xmg_print_stats( const xmg_graph& xmg, std::ostream& os );

unsigned compute_pure_maj_count( const xmg_graph& xmg );

std::vector<unsigned> xmg_compute_levels( const xmg_graph& xmg );

std::vector< std::vector<xmg_node> > xmg_levelize( xmg_graph& xmg );

std::stack<xmg_node> xmg_output_stack( const xmg_graph& xmg );

std::deque<xmg_node> xmg_output_deque( const xmg_graph& xmg );

tt xmg_simulate_cut( const xmg_graph& xmg, xmg_node root, const std::vector<xmg_node>& leafs );

boost::dynamic_bitset<> xmg_output_mask( const xmg_graph& xmg );

/* returns the edge in between parent and child and fails if no such edge exists */
xmg_edge xmg_get_edge( const xmg_graph& xmg, xmg_node parent, xmg_node child );

/* delete all outputs that point to node */
void xmg_delete_po_by_node( xmg_graph& xmg, xmg_node node );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
