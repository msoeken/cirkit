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
 * @file netlist_graphs.hpp
 *
 * @brief Graph data structures for netlists
 *
 * @author Mathias Soeken
 * @since  2.2
 */

#ifndef NETLIST_GRAPHS_HPP
#define NETLIST_GRAPHS_HPP

#include <iostream>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/properties.hpp>

#include <core/utils/graph_utils.hpp>
#include <classical/utils/truth_table_utils.hpp>

namespace boost
{

enum vertex_gate_type_t { vertex_gate_type };
BOOST_INSTALL_PROPERTY(vertex, gate_type);

enum vertex_lut_t { vertex_lut };
BOOST_INSTALL_PROPERTY(vertex, lut);

}

namespace cirkit
{

using boost::adjacency_list;
using boost::default_color_type;
using boost::directedS;
using boost::edge_name_t;
using boost::graph_traits;
using boost::property;
using boost::vecS;
using boost::vertex_color_t;
using boost::vertex_gate_type_t;
using boost::vertex_name_t;
using boost::vertex_lut_t;

enum class gate_type_t { pi, po, gnd, fanout, inv, buf, _and, _or, nand, nor, _xor, xnor, mux, fadd, internal, pwr, dff, dffrs };

/******************************************************************************
 * Simple fan-out graph                                                       *
 ******************************************************************************/

using simple_fanout_vertex_properties_t = property<vertex_name_t, std::string,
                                          property<vertex_gate_type_t, gate_type_t>>;
using simple_fanout_edge_properties_t   = property<edge_name_t, std::pair<std::string, std::string>>;

using simple_fanout_graph_t  = digraph_t<simple_fanout_vertex_properties_t, simple_fanout_edge_properties_t>;
using simple_fanout_vertex_t = vertex_t<simple_fanout_graph_t>;
using simple_fanout_edge_t   = edge_t<simple_fanout_graph_t>;

void write_simple_fanout_graph( std::ostream& os, const simple_fanout_graph_t& g );

/******************************************************************************
 * LUT graph                                                                  *
 ******************************************************************************/

using lut_graph_vertex_properties_t = property<vertex_name_t, std::string,
                                      property<vertex_lut_t, std::pair<unsigned, unsigned long>,
                                      property<vertex_gate_type_t, gate_type_t>>>;
using lut_graph_t  = digraph_t<lut_graph_vertex_properties_t>;
using lut_vertex_t = vertex_t<lut_graph_t>;
using lut_edge_t   = edge_t<lut_graph_t>;

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
