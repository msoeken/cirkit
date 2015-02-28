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

#include <unordered_set>
#include <vector>

#include <boost/dynamic_bitset.hpp>
#include <boost/functional/hash.hpp>
#include <boost/graph/adjacency_list.hpp>

#include <igraph/igraph.h>

#include <core/properties.hpp>
#include <core/utils/graph_utils.hpp>
#include <classical/aig.hpp>

namespace boost
{

enum graph_edge_lookup_t { graph_edge_lookup };
BOOST_INSTALL_PROPERTY(graph, edge_lookup);

}

namespace cirkit
{

using vertex_pair_t    = std::pair<unsigned, unsigned>;

struct edge_lookup_hash_t
{
  inline std::size_t operator()( const vertex_pair_t& p ) const
  {
    std::size_t seed = 0;
    boost::hash_combine( seed, p.first );
    boost::hash_combine( seed, p.second );
    return seed;
  }
};

using edge_lookup_t                 = std::unordered_set<vertex_pair_t, edge_lookup_hash_t>;

using simulation_graph_properties_t = boost::property<boost::graph_edge_lookup_t, edge_lookup_t>;

using simulation_graph              = digraph_t<boost::no_property, boost::no_property, simulation_graph_properties_t>;
using simulation_node               = vertex_t<simulation_graph>;
using simulation_edge               = edge_t<simulation_graph>;

enum class simulation_pattern : unsigned
{
  all_hot = 0x1,
  one_hot = 0x2,
  two_hot = 0x4,
  all_cold = 0x8,
  one_cold = 0x10,
  two_cold = 0x20
};

simulation_graph create_simulation_graph( const aig_graph& aig, const std::vector<boost::dynamic_bitset<>>& sim_vectors,
                                          const properties::ptr& settings = properties::ptr(),
                                          const properties::ptr& statistics = properties::ptr() );

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
