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
#include <boost/graph/topological_sort.hpp>
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
 * a bi-directional graph is too heavy to use.  This allows to precompute
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
 * @brief Precomputes ingoing vertices for directed graphs
 */
template<class G>
inline std::vector<std::vector<vertex_t<G>>> precompute_ingoing_vertices( const G& g )
{
  std::vector<std::vector<vertex_t<G>>> v( boost::num_vertices( g ) );
  for ( const auto& e : boost::make_iterator_range( boost::edges( g ) ) )
  {
    v[boost::target( e, g )].push_back( boost::source( e, g ) );
  }
  return v;
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

template<class Graph, class Fn>
void foreach_topological( const Graph& g, Fn&& f )
{
  std::vector<vertex_t<Graph>> topo( num_vertices( g ) );
  boost::topological_sort( g, topo.begin() );

  for ( const auto& n : topo )
  {
    if ( !f( n ) )
    {
      break;
    }
  }
}

/* some special graphs */
graph_t<> line_graph( unsigned order );
graph_t<> ring_graph( unsigned order );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
