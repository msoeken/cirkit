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

#include <memory>
#include <unordered_map>
#include <vector>

#include <boost/dynamic_bitset.hpp>
#include <boost/functional/hash.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/optional.hpp>

#include <core/properties.hpp>
#include <core/utils/graph_utils.hpp>
#include <classical/aig.hpp>

namespace boost
{

enum graph_edge_lookup_t { graph_edge_lookup };
BOOST_INSTALL_PROPERTY(graph, edge_lookup);

enum graph_meta_t { graph_meta };
BOOST_INSTALL_PROPERTY(graph, meta);

enum vertex_support_t { vertex_support };
BOOST_INSTALL_PROPERTY(vertex, support);

enum vertex_label_t { vertex_label };
BOOST_INSTALL_PROPERTY(vertex, label);

enum vertex_simulation_signature_t { vertex_simulation_signature };
BOOST_INSTALL_PROPERTY(vertex, simulation_signature);

enum edge_label_t { edge_label };
BOOST_INSTALL_PROPERTY(edge, label);

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

namespace detail
{
  using simulation_graph_traits_t          = boost::adjacency_list_traits<boost::vecS, boost::vecS, boost::undirectedS>;
}

using edge_lookup_t                        = std::unordered_map<vertex_pair_t, detail::simulation_graph_traits_t::edge_descriptor, edge_lookup_hash_t>;

struct simulation_graph_meta_t
{
  unsigned num_inputs;
  unsigned num_vectors;
  unsigned num_outputs;

  std::unordered_map<aig_node, detail::simulation_graph_traits_t::vertex_descriptor> port_to_node;
};

using simulation_signature_t               = boost::optional<std::vector<unsigned>>;

/* In order to allow an O(1) lookup for edges in the graph, this
 * edge lookup table is added as a property to the simulation graph
 */
using simulation_graph_properties_t        = boost::property<boost::graph_meta_t, simulation_graph_meta_t,
                                             boost::property<boost::graph_edge_lookup_t, edge_lookup_t>>;

/* In order to allow an O(1) access to the vertex in- and out-degree,
 * they are stored additionally as property maps for the vertices.
 */
using simulation_graph_vertex_properties_t = boost::property<boost::vertex_in_degree_t, unsigned,
                                             boost::property<boost::vertex_out_degree_t, unsigned,
                                             boost::property<boost::vertex_support_t, unsigned,
                                             boost::property<boost::vertex_name_t, std::string,
                                             boost::property<boost::vertex_label_t, unsigned,
                                             boost::property<boost::vertex_simulation_signature_t, simulation_signature_t>>>>>>;

using simulation_graph_edge_properties_t   = boost::property<boost::edge_label_t, unsigned>;

using simulation_graph                     = graph_t<simulation_graph_vertex_properties_t,
                                                     simulation_graph_edge_properties_t,
                                                     simulation_graph_properties_t>;
using simulation_node                      = vertex_t<simulation_graph>;
using simulation_edge                      = edge_t<simulation_graph>;

/**
 * This function does not assign labels
 */
simulation_graph create_simulation_graph( const aig_graph& aig, const std::vector<boost::dynamic_bitset<>>& sim_vectors,
                                          const properties::ptr& settings = properties::ptr(),
                                          const properties::ptr& statistics = properties::ptr() );

/**
 * @param types The values in this array determine which k-cold and k-hot vectors should
 *              be generated.  Let j be an element in that vector, then k is obtained by
 *              k := j div 2 (where div is integer division) and the type is hot if and
 *              only if j % 2 == 1.  As an example, 2h = 5 and 1c = 2.
 * @param partition If not nullptr then the number of patterns for each selector is written into it.
 */
std::vector<boost::dynamic_bitset<>> create_simulation_vectors( unsigned width, const std::vector<unsigned>& types,
                                                                std::vector<unsigned>* partition = nullptr );

simulation_graph create_simulation_graph( const aig_graph& aig, const std::vector<unsigned>& types,
                                          const properties::ptr& settings = properties::ptr(),
                                          const properties::ptr& statistics = properties::ptr() );

std::vector<simulation_signature_t::value_type> compute_simulation_signatures( const aig_graph& aig, unsigned maxk = 2u );

/******************************************************************************
 * simulation_graph_wrapper                                                   *
 ******************************************************************************/
/**
 * If FAST_EDGE_ACCESS is defined, edge directions and edge labels are stored
 * in an adjacency matrix, which increases performance but also memory usage.
 */
//#define FAST_EDGE_ACCESS

class simulation_graph_wrapper
{
public:
  using vertex_range_t    = boost::iterator_range<boost::graph_traits<simulation_graph>::vertex_iterator>;
  using edge_range_t      = boost::iterator_range<boost::graph_traits<simulation_graph>::edge_iterator>;
  using adjacency_range_t = boost::iterator_range<boost::graph_traits<simulation_graph>::adjacency_iterator>;

public:
  simulation_graph_wrapper( const aig_graph& g,
                            const std::vector<unsigned>& types,
                            bool support_edges,
                            const boost::optional<unsigned>& simulation_signatures );

  inline const simulation_graph& sim_graph() const                       { return graph; }

  inline unsigned num_inputs() const                                     { return boost::get_property( graph, boost::graph_meta ).num_inputs; }
  inline unsigned num_vectors() const                                    { return boost::get_property( graph, boost::graph_meta ).num_vectors; }
  inline unsigned num_outputs() const                                    { return boost::get_property( graph, boost::graph_meta ).num_outputs; }

  inline unsigned size() const                                           { return boost::num_vertices( graph ); }
  inline unsigned degree( unsigned u ) const                             { return boost::out_degree( u, graph ); }
  inline unsigned in_degree( unsigned u ) const                          { return vertex_in_degree[u]; }
  inline unsigned out_degree( unsigned u ) const                         { return vertex_out_degree[u]; }
  inline unsigned support( unsigned u ) const                            { return vertex_support[u]; }
  inline unsigned label( unsigned u ) const                              { return vertex_label[u]; }
  inline simulation_signature_t simulation_signature( unsigned u ) const { return vertex_simulation_signature[u]; }
  inline const std::string& name( unsigned u ) const                     { return vertex_name[u]; }

  inline unsigned port_to_node( const aig_node& port ) const             { return boost::get_property( graph, boost::graph_meta ).port_to_node.at( port ); }

  inline vertex_range_t    vertices() const             { return boost::make_iterator_range( boost::vertices( graph ) ); }
  inline edge_range_t      edges() const                { return boost::make_iterator_range( boost::edges( graph ) ); }
  inline adjacency_range_t adjacent( unsigned u ) const { return boost::make_iterator_range( boost::adjacent_vertices( u, graph ) ); }

  inline unsigned edge_direction( unsigned u, unsigned v ) const
  {
#ifdef FAST_EDGE_ACCESS
    return vedge_direction[u][v];
#else
    const auto it = vedge_direction[u].find( v );
    return ( it == vedge_direction[u].end() ) ? 0u : it->second;
#endif
  }

  inline unsigned edge_label( unsigned u, unsigned v ) const
  {
#ifdef FAST_EDGE_ACCESS
    return vedge_label[u][v];
#else
    const auto it = vedge_label[u].find( v );
    return ( it == vedge_label[u].end() ) ? 0u : it->second;
#endif
  }

private:
  simulation_graph graph;

  boost::property_map<simulation_graph, boost::vertex_label_t>::type                vertex_label;
  boost::property_map<simulation_graph, boost::vertex_in_degree_t>::type            vertex_in_degree;
  boost::property_map<simulation_graph, boost::vertex_out_degree_t>::type           vertex_out_degree;
  boost::property_map<simulation_graph, boost::vertex_support_t>::type              vertex_support;
  boost::property_map<simulation_graph, boost::vertex_name_t>::type                 vertex_name;
  boost::property_map<simulation_graph, boost::vertex_simulation_signature_t>::type vertex_simulation_signature;
  boost::property_map<simulation_graph, boost::edge_label_t>::type                  medge_label;

#ifdef FAST_EDGE_ACCESS
  std::vector<std::vector<int>>                                                     vedge_label;
  std::vector<std::vector<int>>                                                     vedge_direction;
#else
  /* there is no other way right now to efficiently handle this */
  std::vector<std::unordered_map<unsigned, unsigned>>                               vedge_label;
  std::vector<std::unordered_map<unsigned, unsigned>>                               vedge_direction;
#endif
};

bool compatible_simulation_signatures( const simulation_graph_wrapper& pg, const simulation_graph_wrapper& tg,
                                       const simulation_node& u, const simulation_node& v,
                                       unsigned maxk );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
