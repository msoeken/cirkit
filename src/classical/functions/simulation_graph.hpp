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
 * @file simulation_graph.hpp
 *
 * @brief Simulation graph
 *
 * @author Mathias Soeken
 * @since  2.0
 */

#ifndef SIMULATION_GRAPH_HPP
#define SIMULATION_GRAPH_HPP

#include <vector>

#include <boost/dynamic_bitset.hpp>
#include <boost/graph/adjacency_list.hpp>

#include <igraph/igraph.h>

#include <core/properties.hpp>
#include <classical/aig.hpp>

namespace cirkit
{

typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS> simulation_graph;

typedef boost::graph_traits<simulation_graph>::vertex_descriptor simulation_node;
typedef boost::graph_traits<simulation_graph>::edge_descriptor simulation_edge;

enum class simulation_pattern : unsigned
{
  all_hot = 0x1,
  one_hot = 0x2,
  two_hot = 0x4,
  all_cold = 0x8,
  one_cold = 0x10,
  two_cold = 0x20
};

void create_simulation_graph( simulation_graph& g, const aig_graph& aig, const std::vector<boost::dynamic_bitset<>>& sim_vectors, properties::ptr settings = properties::ptr() );

/**
 * @param partition If not nullptr then the number of patterns for each selector is written into it.
 */
void create_simulation_vectors( std::vector<boost::dynamic_bitset<>>& sim_vectors, unsigned width, unsigned selector,
                                std::vector<unsigned>* partition = nullptr );

igraph_t simulation_graph_to_igraph( const simulation_graph& g );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
