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
 * @file bdd_utils.hpp
 *
 * @brief Some helper functions for BDDs
 *
 * @author Mathias Soeken
 * @since  2.0
 */

#ifndef BDD_UTILS_HPP
#define BDD_UTILS_HPP

#include <vector>

#include <cuddObj.hh>

namespace cirkit
{

BDD make_cube( Cudd& manager, const std::vector<BDD>& vars );

bool is_selfdual( const Cudd& manager, const BDD& f );
bool is_monotone( const Cudd& manager, const BDD& f );
bool is_horn( const Cudd& manager, const BDD& f, const BDD& g, const BDD& h );
bool is_horn( const Cudd& manager, const BDD& f );
bool is_unate( const Cudd& manager, const BDD& f, std::vector<int>& ps );

std::vector<unsigned> level_sizes( DdManager* manager, const std::vector<DdNode*>& fs );
std::vector<unsigned> level_sizes( const Cudd& manager, const std::vector<BDD>& fs );

/**
 * Computes the maximum fanout of a node without respecting the
 * complemented edges.  This function should probably be moved
 * to some memristor implementation code.
 */
unsigned maximum_fanout( DdManager* manager, const std::vector<DdNode*>& fs );
unsigned maximum_fanout( const Cudd& manager, const std::vector<BDD>& fs );

unsigned count_complement_edges( DdManager* manager, const std::vector<DdNode*>& fs );
unsigned count_complement_edges( const Cudd& manager, const std::vector<BDD>& fs );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
