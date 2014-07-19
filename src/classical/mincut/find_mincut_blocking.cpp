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

#include "find_mincut_blocking.hpp"

#include <fstream>
#include <list>

#include <boost/assign/std/list.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/graph/boykov_kolmogorov_max_flow.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/optional.hpp>

using namespace boost::assign;

namespace cirkit
{

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

std::pair<mc_edge_t, mc_edge_t> add_edges( const mc_vertex_t& s, const mc_vertex_t& t, double capacity, mc_graph_t& graph )
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

std::pair<mc_vertex_t, mc_vertex_t> create_mincut_graph_with_blocking( mc_graph_t& graph, const aig_graph& aig, const std::vector<aig_node>& blocked_nodes )
{
  const auto& graph_info = boost::get_property( aig, boost::graph_name );
  auto namemap = get( boost::edge_name, graph );

  /* A map to store AIG node to MC graph node */
  std::map<aig_node, std::pair<mc_vertex_t, mc_vertex_t>> node_map;

  /* Source and target */
  mc_vertex_t source = boost::add_vertex( graph );
  mc_vertex_t target = boost::add_vertex( graph );

  /* Copy nodes */
  for ( const aig_node& node : boost::make_iterator_range( boost::vertices( aig ) ) )
  {
    mc_vertex_t s = boost::add_vertex( graph );
    mc_vertex_t t = boost::add_vertex( graph );

    bool is_blocked = boost::find( blocked_nodes, node ) != blocked_nodes.end();
    mc_edge_t e = add_edges( s, t, is_blocked ? std::numeric_limits<double>::infinity() : 1.0, graph ).first;
    namemap[e].original_node = node;

    node_map[node] = std::make_pair( s, t );
  }

  /* Copy edges */
  for ( const aig_edge& edge : boost::make_iterator_range( boost::edges( aig ) ) )
  {
    const mc_vertex_t& s = node_map[boost::source( edge, aig )].second;
    const mc_vertex_t& t = node_map[boost::target( edge, aig )].first;

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

bool find_mincut_blocking( std::list<std::list<aig_node>>& cuts, aig_graph& aig, unsigned count, properties::ptr settings, properties::ptr statistics )
{
  /* settings */
  bool verbose = get( settings, "verbose", false );

  std::vector<aig_node> blocked_nodes;

  for ( unsigned i = 0u; i < count; ++i )
  {
    if ( verbose )
    {
      std::cout << "[I] find min cut " << i << std::endl;
    }

    mc_graph_t graph;
    mc_vertex_t source, target;

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
          if ( name[e].original_node )
          {
            cut += *( name[e].original_node );
            blocked_nodes += *( name[e].original_node );
          }
          else
          {
            if ( verbose )
            {
              std::cout << "[W] no node assigned to " << e << std::endl;
            }
          }
        }
      }
    }

    cuts += cut;
  }

  return true;
}

mincut_by_node_func find_mincut_blocking_func( properties::ptr settings, properties::ptr statistics )
{
  mincut_by_node_func f = [&settings, &statistics]( std::list<std::list<aig_node>>& cuts, aig_graph& aig, unsigned count ) {
    return find_mincut_blocking( cuts, aig, count, settings, statistics );
  };
  f.init( settings, statistics );
  return f;
}


}

// Local Variables:
// c-basic-offset: 2
// End:
