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
 * @file write_graphviz.hpp
 *
 * @brief Use graphviz to render and print the graph
 *
 * @author Heinz Riener
 * @since  2.0
 */

#ifndef WRITE_GRAPHVIZ_HPP
#define WRITE_GRAPHVIZ_HPP

#if ADDON_GRAPHVIZ

#include <classical/aig.hpp>
#include <classical/graphviz.hpp>
#include <string>
#include <map>

namespace cirkit
{

void compute_graphviz_layout( gv_graph& gv, const aig_graph& aig, const std::string& layout_algorithm, const std::string& render_format,
                              std::map< aig_node, gv_node >& node_map, std::map< aig_edge, gv_edge >& edge_map );

void write_graphviz( const aig_graph& aig, const std::string& layout_algorithm, const std::string& graphviz_renderer, std::ostream& os );
void write_graphviz( const aig_graph& aig, const std::string& layout_algorithm, const std::string& render_format,
                     std::map< aig_node, gv_node >& node_map, std::map< aig_edge, gv_edge >& edge_map, std::ostream& os );

}

#endif

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
