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
 * @file xmg_mffc.hpp
 *
 * @brief Compute MFFCs
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef XMG_MFFC_HPP
#define XMG_MFFC_HPP

#include <map>
#include <vector>

#include <classical/xmg/xmg.hpp>

namespace cirkit
{

unsigned xmg_compute_mffc( xmg_graph& xmg, xmg_node n, std::vector<xmg_node>& support );
std::map<xmg_node, std::vector<xmg_node>> xmg_mffcs( xmg_graph& xmg );

/* counts nodes including the root, but excluding the leafs */
unsigned xmg_mffc_size( const xmg_graph& xmg, xmg_node n, const std::vector<xmg_node>& support );
/* returns nodes including the root, but excluding the leafs */
std::vector<xmg_node> xmg_mffc_cone( const xmg_graph& xmg, xmg_node n, const std::vector<xmg_node>& support );

/* check if curr is contained in the mffc defined by the pair (root,support) */
bool xmg_mffc_contains( const xmg_graph& xmg, xmg_node root, const std::vector<xmg_node>& support, xmg_node curr );
/* compute the number of nodes of the subcone (within mffcs) rooted by curr */
unsigned xmg_mffc_tipsize( const xmg_graph& xmg, const std::map<xmg_node, std::vector<xmg_node>> mffcs, xmg_node curr );

/* mark all nodes of a mffc defined by the pair (root,support) on xmg */
void xmg_mffc_mark( xmg_graph& xmg, xmg_node root, const std::vector<xmg_node>& support );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
