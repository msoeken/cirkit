/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2014  University of Bremen
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

#include "find_mincut_by_nodes.hpp"

#include <fstream>
#include <list>

#include <boost/assign/std/list.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/format.hpp>
#include <boost/graph/boykov_kolmogorov_max_flow.hpp>
#include <boost/graph/filtered_graph.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/optional.hpp>

using namespace boost::assign;

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

typedef boost::adjacency_list_traits<boost::vecS, boost::vecS, boost::bidirectionalS> mc_traits_t;
typedef boost::property<boost::vertex_color_t, boost::default_color_type,
        boost::property<boost::vertex_distance_t, long,
        boost::property<boost::vertex_predecessor_t, mc_traits_t::edge_descriptor,
        boost::property<boost::vertex_name_t, boost::optional<aig_node>>>>> mc_vertex_properties_t;
typedef boost::property<boost::edge_capacity_t, double,
        boost::property<boost::edge_residual_capacity_t, double,
        boost::property<boost::edge_reverse_t, mc_traits_t::edge_descriptor,
        boost::property<boost::edge_name_t, boost::optional<aig_node>>>>> mc_edge_properties_t;
typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, mc_vertex_properties_t, mc_edge_properties_t> mc_graph_t;

typedef boost::graph_traits<mc_graph_t>::vertex_descriptor mc_node_t;
typedef boost::graph_traits<mc_graph_t>::edge_descriptor mc_edge_t;


/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

std::pair<mc_edge_t, mc_edge_t> add_edges( const mc_node_t& s, const mc_node_t& t, double capacity, mc_graph_t& graph )
{
  auto capacitymap = get( boost::edge_capacity, graph );
  auto reversemap  = get( boost::edge_reverse,  graph );

  auto edge  = boost::add_edge( s, t, graph ).first;
  auto redge = boost::add_edge( t, s, graph ).first;

  capacitymap[edge] = capacity;
  capacitymap[redge] = 0.0;
  reversemap[edge] = redge;
  reversemap[redge] = edge;

  return std::make_pair( edge, redge );
}

mc_edge_t add_edge( const mc_node_t& s, const mc_node_t& t, double capacity, mc_graph_t& graph )
{
  auto capacitymap = get( boost::edge_capacity, graph );
  auto edge = boost::add_edge( s, t, graph ).first;

  capacitymap[edge] = capacity;

  return edge;
}

void add_reverse_edges( mc_graph_t& graph )
{
  auto capacitymap = get( boost::edge_capacity, graph );
  auto reversemap  = get( boost::edge_reverse,  graph );

  std::list<mc_edge_t> store_edges;
  boost::push_back( store_edges, boost::make_iterator_range( edges( graph ) ) );

  for ( const auto& edge : store_edges )
  {
    auto redge = boost::add_edge( boost::target( edge, graph ), boost::source( edge, graph ), graph ).first;
    capacitymap[redge] = 0.0;
    reversemap[edge] = redge;
    reversemap[redge] = edge;
  }
}

std::pair<mc_node_t, mc_node_t> create_mincut_graph_with_blocking( mc_graph_t& graph, const aig_graph& aig, const std::vector<aig_node>& blocked_nodes )
{
  const auto& graph_info = boost::get_property( aig, boost::graph_name );
  auto namemap = get( boost::edge_name, graph );

  /* A map to store AIG node to MC graph node */
  std::map<aig_node, std::pair<mc_node_t, mc_node_t>> node_map;

  /* Source and target */
  mc_node_t source = boost::add_vertex( graph );
  mc_node_t target = boost::add_vertex( graph );

  /* Copy nodes */
  for ( const aig_node& node : boost::make_iterator_range( boost::vertices( aig ) ) )
  {
    mc_node_t s = boost::add_vertex( graph );
    mc_node_t t = boost::add_vertex( graph );

    bool is_blocked = boost::find( blocked_nodes, node ) != blocked_nodes.end();
    mc_edge_t e = add_edges( s, t, is_blocked ? std::numeric_limits<double>::infinity() : 1.0, graph ).first;
    namemap[e] = node;

    node_map[node] = std::make_pair( s, t );
  }

  /* Copy edges */
  for ( const aig_edge& edge : boost::make_iterator_range( boost::edges( aig ) ) )
  {
    const mc_node_t& s = node_map[boost::source( edge, aig )].second;
    const mc_node_t& t = node_map[boost::target( edge, aig )].first;

    add_edges( s, t, std::numeric_limits<double>::infinity(), graph );
  }

  /* Connect source with outputs */
  for ( const auto& output : graph_info.outputs )
  {
    add_edges( source, node_map[output.first.first].first, std::numeric_limits<double>::infinity(), graph );
  }

  /* Connect target with inputs */
  for ( const auto& input : graph_info.inputs )
  {
    add_edges( node_map[input].second, target, std::numeric_limits<double>::infinity(), graph );
  }

  /* Connect target (and maybe source) with constant */
  add_edges( node_map[graph_info.constant].second, target, std::numeric_limits<double>::infinity(), graph );
  if ( !graph_info.constant_used )
  {
    add_edges( source, node_map[graph_info.constant].first, std::numeric_limits<double>::infinity(), graph );
  }

  return std::make_pair( source, target );
}

template<typename Graph>
struct has_color
{
  has_color() {}
  has_color( const Graph& graph, unsigned color ) : graph( graph ), color( color ) {}

  bool operator()( const aig_node& n ) const
  {
    return boost::get( boost::vertex_color, graph )[n] == color;
  }

  bool operator()( const aig_edge& e ) const
  {
    const auto& map = boost::get( boost::vertex_color, graph );
    return ( map[boost::source( e, graph )] == color ) &&
           ( map[boost::target( e, graph )] == color );
  }

private:
  const Graph& graph;
  unsigned color;
};

std::pair<mc_node_t, mc_node_t> create_mincut_graph_with_splitting( mc_graph_t& graph, const aig_graph& aig, unsigned color )
{
  has_color<aig_graph> filter( aig, color );
  boost::filtered_graph<aig_graph, has_color<aig_graph>, has_color<aig_graph>> fg( aig, filter, filter );

  auto namemap  = get( boost::edge_name, graph   );
  auto vnamemap = get( boost::vertex_name, graph );

  /* A map to store AIG node to MC graph node */
  std::map<aig_node, std::pair<mc_node_t, mc_node_t>> node_map;

  /* Source and target */
  mc_node_t source = boost::add_vertex( graph );
  mc_node_t target = boost::add_vertex( graph );

  /* Copy nodes */
  for ( const aig_node& node : boost::make_iterator_range( boost::vertices( fg ) ) )
  {
    mc_node_t s = boost::add_vertex( graph );
    mc_node_t t = boost::add_vertex( graph );
    vnamemap[s] = node;
    vnamemap[t] = node;

    mc_edge_t e = add_edge( s, t, 1.0, graph );
    namemap[e] = node;

    node_map[node] = std::make_pair( s, t );
  }

  /* Copy edges */
  for ( const aig_edge& edge : boost::make_iterator_range( boost::edges( fg ) ) )
  {
    const mc_node_t& s = node_map[boost::source( edge, aig )].second;
    const mc_node_t& t = node_map[boost::target( edge, aig )].first;

    add_edge( s, t, std::numeric_limits<double>::infinity(), graph );
  }

  /* Source and target */
  for ( const auto& vertex : boost::make_iterator_range( boost::vertices( graph ) ) )
  {
    if ( vertex == source || vertex == target ) continue;

    if ( in_degree( vertex, graph ) == 0 )
    {
      add_edge( source, vertex, std::numeric_limits<double>::infinity(), graph );
    }
    if ( out_degree( vertex, graph ) == 0 )
    {
      add_edge( vertex, target, std::numeric_limits<double>::infinity(), graph );
    }
  }

  add_reverse_edges( graph );

  /* write graph to dot */
  if ( true )
  {
    std::filebuf fb;
    fb.open( boost::str( boost::format( "/tmp/graph-splitted-%d" ) % color ).c_str(), std::ios::out );
    std::ostream os( &fb );
    write_graphviz( os, graph );
    fb.close();
  }

  return std::make_pair( source, target );
}

void find_mincut_by_nodes_with_blocking( const aig_graph& aig, unsigned count, std::list<std::list<aig_node>>& cuts, const find_mincut_by_nodes_settings& settings )
{
  std::vector<aig_node> blocked_nodes;

  for ( unsigned i = 0u; i < count; ++i )
  {
    if ( settings.verbose )
    {
      std::cout << "[I] find min cut " << i << std::endl;
    }

    mc_graph_t graph;
    mc_node_t source, target;

    boost::tie( source, target ) = create_mincut_graph_with_blocking( graph, aig, blocked_nodes );

    boykov_kolmogorov_max_flow( graph, source, target );

    auto capacity = boost::get( boost::edge_capacity, graph );
    auto color    = boost::get( boost::vertex_color,  graph );
    auto name     = boost::get( boost::edge_name,     graph );

    std::list<aig_node> cut;

    for ( const auto& e : boost::make_iterator_range( edges( graph ) ) )
    {
      if ( capacity[e] > 0 )
      {
        if ( color[boost::source(e, graph)] != color[boost::target(e, graph)] )
        {
          if ( name[e] )
          {
            cut += *name[e];
            blocked_nodes += *name[e];
          }
          else
          {
            if ( settings.verbose )
            {
              std::cout << "[W] no node assigned to " << e << std::endl;
            }
          }
        }
      }
    }

    cuts += cut;
  }
}

void find_mincut_by_nodes_with_splitting( aig_graph& aig, unsigned count, std::list<std::list<aig_node>>& cuts, const find_mincut_by_nodes_settings& settings )
{
  /* color nodes */
  unsigned max_color = 0u;
  auto aig_color = boost::get( boost::vertex_color, aig );
  for ( const auto& v : boost::make_iterator_range( vertices( aig ) ) )
  {
    aig_color[v] = max_color;
  }

  /* find cuts */
  for ( unsigned i = 0u; i < count; ++i )
  {
    if ( settings.verbose )
    {
      std::cout << "[I] find min cut " << i << std::endl;
    }

    mc_graph_t graph;
    mc_node_t source, target;

    boost::tie( source, target ) = create_mincut_graph_with_splitting( graph, aig, i );

    boykov_kolmogorov_max_flow( graph, source, target );

    auto capacity = boost::get( boost::edge_capacity, graph );
    auto color    = boost::get( boost::vertex_color,  graph );
    auto name     = boost::get( boost::edge_name,     graph );
    auto vname    = boost::get( boost::vertex_name,   graph );

    std::list<aig_node> cut;

    for ( const auto& e : boost::make_iterator_range( edges( graph ) ) )
    {
      if ( capacity[e] > 0 )
      {
        if ( color[boost::source(e, graph)] != color[boost::target(e, graph)] )
        {
          if ( name[e] )
          {
            cut += *name[e];
          }
          else
          {
            if ( settings.verbose )
            {
              std::cout << "[W] no node assigned to " << e << std::endl;
            }
          }
        }
      }
    }

    for ( const auto& v : boost::make_iterator_range( vertices( graph ) ) )
    {
      if ( vname[v] )
      {
        /* node is not in the cut */
        if ( boost::find( cut, *vname[v] ) == cut.end() )
        {
          //assert( color[v] == boost::white_color || color[v] == boost::black_color );
          aig_color[*vname[v]] = max_color + 1u + ( color[v] == boost::white_color ? 0u : 1u );
        }
      }
      else
      {
        assert( v == source || v == target );
      }
    }

    cuts += cut;
    max_color += 2u;
  }
}


/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

void find_mincut_by_nodes( aig_graph& aig, unsigned count, std::list<std::list<aig_node>>& cuts, const find_mincut_by_nodes_settings& settings )
{
  if ( settings.mode == splitting )
  {
    find_mincut_by_nodes_with_splitting( aig, count, cuts, settings );
  }
  else if ( settings.mode == blocking )
  {
    find_mincut_by_nodes_with_blocking( aig, count, cuts, settings );
  }
  else
  {
    /* mode not found */
    assert( 0 );
  }
}


}

// Local Variables:
// c-basic-offset: 2
// End:
