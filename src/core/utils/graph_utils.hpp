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
 * @file graph_utils.hpp
 *
 * @brief Some helper functions for graphs
 *
 * @author Mathias Soeken
 * @since  2.2
 */

#ifndef GRAPH_UTILS_HPP
#define GRAPH_UTILS_HPP

#include <map>
#include <vector>

#include <boost/assign/std/vector.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/copy.hpp>
#include <boost/graph/filtered_graph.hpp>
#include <boost/range/algorithm.hpp>

using namespace boost::assign;

namespace boost
{

enum vertex_annotation_t { vertex_annotation };
BOOST_INSTALL_PROPERTY(vertex, annotation);

enum edge_complement_t { edge_complement };
BOOST_INSTALL_PROPERTY(edge, complement);

}

namespace cirkit
{

/* These are some graph type aliases that make definition of graphs a bit less verbose. */

/**
 * @brief Digraph alias
 *
 * Since I often use directed graphs with boost::vecS/boost::vecS access, there is
 * an alias digraph_t for that purpose to which vertex, edge, and graph properties can
 * be provided.
 */
template<class VertexProperty = boost::no_property,
         class EdgeProperty = boost::no_property,
         class GraphProperty = boost::no_property>
using digraph_t = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, VertexProperty, EdgeProperty, GraphProperty>;

using digraph_traits_t = boost::adjacency_list_traits<boost::vecS, boost::vecS, boost::directedS>;

/**
 * @brief Graph alias
 *
 * Similar to digraph_t
 */
template<class VertexProperty = boost::no_property,
         class EdgeProperty = boost::no_property,
         class GraphProperty = boost::no_property>
using graph_t = boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, VertexProperty, EdgeProperty, GraphProperty>;

/**
 * @brief Vertex alias
 *
 * Less verbose vertex type alias
 */
template<class G>
using vertex_t = typename boost::graph_traits<G>::vertex_descriptor;

/**
 * @brief Edge alias
 *
 * Less verbose edge type alias
 */
template<class G>
using edge_t = typename boost::graph_traits<G>::edge_descriptor;

/**
 * @brief Precomputes ingoing edges for directed graphs
 *
 * Directed graphs have no access to their ingoing edges, but sometimes
 * a bi-directional graph is too heavey to use.  This allows to precompute
 * the ingoing edges once in O(|E|) and store them in a map.
 */
template<class G>
inline std::map<vertex_t<G>, std::vector<edge_t<G>>> precompute_ingoing_edges( const G& g )
{
  std::map<vertex_t<G>, std::vector<edge_t<G>>> m;
  for ( const auto& e : boost::make_iterator_range( boost::edges( g ) ) )
  {
    m[boost::target( e, g )] += e;
  }
  return m;
}

/**
 * @brief Precomputes in-degrees for directed graphs
 *
 * @see precompute_ingoing_edges
 */
template<class G>
inline std::vector<unsigned> precompute_in_degrees( const G& g )
{
  std::vector<unsigned> v( boost::num_vertices( g ), 0u );
  for ( const auto& e : boost::make_iterator_range( boost::edges( g ) ) )
  {
    v[boost::target( e, g )]++;
  }
  return v;
}

/**
 * @brief Predicate to keep vertices that have a certain property
 */
template<class PropertyMap>
struct vertex_has_property
{
  using value_type = typename boost::property_traits<PropertyMap>::value_type;

  vertex_has_property() {}
  vertex_has_property( const PropertyMap& map, const value_type& value )
    : map( &map ), value( &value ) {}

  template<typename Vertex>
  bool operator()( const Vertex& v ) const
  {
    return boost::get( *map, v ) == *value;
  }

private:
  PropertyMap const* map;
  value_type const* value;
};

/**
 * @brief Predicate to keep vertices that do not have a certain property
 */
template<class PropertyMap>
struct vertex_has_not_property
{
  using value_type = typename boost::property_traits<PropertyMap>::value_type;

  vertex_has_not_property() {}
  vertex_has_not_property( const PropertyMap& map, const value_type& value )
    : map( &map ), value( &value ) {}

  template<typename Vertex>
  bool operator()( const Vertex& v ) const
  {
    return boost::get( *map, v ) != *value;
  }

private:
  PropertyMap const* map;
  value_type const* value;
};

/**
 * @brief Predicate to keep edges whose adjacent vertices have a certain property
 */
template<class Graph, class PropertyMap>
struct edge_has_property
{
  using value_type = typename boost::property_traits<PropertyMap>::value_type;

  edge_has_property() {}
  edge_has_property( const Graph& g, const PropertyMap& map, const value_type& value )
    : g( &g ), map( &map ), value( &value ) {}

  template<typename Edge>
  bool operator()( const Edge& e ) const
  {
    const auto& s = boost::source( e, *g );
    const auto& t = boost::target( e, *g );
    return ( boost::get( *map, s ) == *value ) && ( boost::get( *map, t ) == *value );
  }

private:
  Graph const* g;
  PropertyMap const* map;
  value_type const* value;
};

/**
 * @brief Predicate to keep edges whose adjacent vertices do not have a certain property
 */
template<class Graph, class PropertyMap>
struct edge_has_not_property
{
  using value_type = typename boost::property_traits<PropertyMap>::value_type;

  edge_has_not_property() {}
  edge_has_not_property( const Graph& g, const PropertyMap& map, const value_type& value )
    : g( &g ), map( &map ), value( &value ) {}

  template<typename Edge>
  bool operator()( const Edge& e ) const
  {
    const auto& s = boost::source( e, *g );
    const auto& t = boost::target( e, *g );
    return ( boost::get( *map, s ) != *value ) || ( boost::get( *map, t ) != *value );
  }

private:
  Graph const* g;
  PropertyMap const* map;
  value_type const* value;
};

template<class Dest, class Source>
Dest copy_from_filtered( const Source& source,
                         std::vector<typename boost::graph_traits<Dest>::vertex_descriptor>& copy_map )
{
  using source_vertex_t = typename boost::graph_traits<Source>::vertex_descriptor;
  using dest_vertex_t   = typename boost::graph_traits<Dest>::vertex_descriptor;
  using index_map_t     = typename boost::property_map<Source, boost::vertex_index_t>::type;
  using iso_map_t       = boost::iterator_property_map<typename std::vector<dest_vertex_t>::iterator, index_map_t, source_vertex_t, source_vertex_t&>;

  Dest dest;
  copy_map.resize( boost::num_vertices( source ) );
  iso_map_t copy_imap( copy_map.begin(), boost::get( boost::vertex_index, source ) );
  boost::copy_graph( source, dest, boost::orig_to_copy( copy_imap ) );
  return dest;
}

template<class Graph>
std::vector<vertex_t<Graph>> add_vertices( Graph& g, unsigned n )
{
  std::vector<vertex_t<Graph>> nodes( n );
  boost::generate( nodes, [&]() { return boost::add_vertex( g ); } );
  return nodes;
}

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
