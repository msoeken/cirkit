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

#include <core/cube.hpp>

namespace cirkit
{

using bdd_function_t = std::pair<Cudd, std::vector<BDD>>;

BDD make_cube( Cudd& manager, const std::vector<BDD>& vars );
BDD make_cube( Cudd& manager, const std::string& cube );

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

/**
 * Computes a BDD for sum( vars ) = k
 */
BDD make_eq( const Cudd& manager, const std::vector<BDD>& vars, unsigned k );

std::vector<DdNode*> bdd_copy( DdManager* mgr_from, const std::vector<DdNode*>& from, DdManager* mgr_to, std::vector<unsigned>& index_map );

std::vector<BDD> bdd_copy( const Cudd& mgr_from, const std::vector<BDD>& from, const Cudd& mgr_to, std::vector<unsigned>& index_map );

bdd_function_t compute_characteristic( const bdd_function_t& bdd, bool inputs_first );

cube_vec_t bdd_to_cubes( DdManager* manager, DdNode* f );
cube_vec_t bdd_to_cubes( const Cudd& manager, BDD f );

/******************************************************************************
 * new BDD operations                                                         *
 ******************************************************************************/

DdNode* bdd_up( DdManager* manager, DdNode* f );
BDD bdd_up( Cudd& manager, const BDD& f );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
