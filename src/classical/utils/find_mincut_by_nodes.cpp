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

#include <boost/assign/std/list.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/graph/boykov_kolmogorov_max_flow.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/iterator_range.hpp>

using namespace boost::assign;

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

typedef boost::adjacency_list_traits<boost::vecS, boost::vecS, boost::directedS> mc_traits_t;
typedef boost::property<boost::vertex_color_t, boost::default_color_type,
        boost::property<boost::vertex_distance_t, long,
        boost::property<boost::vertex_predecessor_t, mc_traits_t::edge_descriptor>>> mc_vertex_properties_t;
typedef boost::property<boost::edge_capacity_t, double,
        boost::property<boost::edge_residual_capacity_t, double,
        boost::property<boost::edge_reverse_t, mc_traits_t::edge_descriptor,
        boost::property<boost::edge_name_t, aig_node>>>> mc_edge_properties_t;
typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, mc_vertex_properties_t, mc_edge_properties_t> mc_graph_t;

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

std::pair<mc_node_t, mc_node_t> create_mincut_graph( mc_graph_t& graph, const aig_graph& aig, const std::vector<aig_node>& blocked_nodes )
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

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

void find_mincut_by_nodes( const aig_graph& aig, unsigned count, std::list<std::list<aig_node>>& cuts )
{
  std::vector<aig_node> blocked_nodes;

  for ( unsigned i = 0u; i < count; ++i )
  {
    mc_graph_t graph;
    mc_node_t source, target;

    boost::tie( source, target ) = create_mincut_graph( graph, aig, blocked_nodes );

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
          cut += name[e];
          blocked_nodes += name[e];
        }
      }
    }

    cuts += cut;
  }
}

}

// Local Variables:
// c-basic-offset: 2
// End:
