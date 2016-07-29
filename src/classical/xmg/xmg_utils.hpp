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

std::stack<xmg_node> xmg_output_stack( const xmg_graph& xmg );

std::deque<xmg_node> xmg_output_deque( const xmg_graph& xmg );

tt xmg_simulate_cut( const xmg_graph& xmg, xmg_node root, const std::vector<xmg_node>& leafs );

boost::dynamic_bitset<> xmg_output_mask( const xmg_graph& xmg );

/* returns the edge in between parent and child and fails if no such edge exists */
xmg_edge xmg_get_edge( const xmg_graph& xmg, xmg_node parent, xmg_node child );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
