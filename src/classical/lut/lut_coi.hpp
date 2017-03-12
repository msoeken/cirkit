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
 * @file lut_coi.hpp
 *
 * @brief Compute COI of nodes
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef LUT_COI_HPP
#define LUT_COI_HPP

#include <classical/lut/lut_graph.hpp>

#include <vector>

namespace cirkit
{

/* the function returns the number of nodes in the COI and if primary_inputs is not null, it puts the PIs in order of appearance */
unsigned lut_compute_coi( const lut_graph& graph, const lut_vertex_t& start_node, std::vector<lut_vertex_t>* primary_inputs = nullptr );

boost::dynamic_bitset<> lut_compute_coi_as_bitset( const lut_graph& graph, const lut_vertex_t& start_node );

std::vector<lut_vertex_t> lut_coi_topological_nodes( const lut_graph& graph );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
