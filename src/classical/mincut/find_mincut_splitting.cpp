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

#include "find_mincut_splitting.hpp"

#include <fstream>
#include <list>

#include <boost/assign/std/list.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/format.hpp>
#include <boost/graph/boykov_kolmogorov_max_flow.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
#include <boost/range/iterator_range.hpp>

using namespace boost::assign;

namespace cirkit
{

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

mc_edge_t add_edge( const mc_vertex_t& s, const mc_vertex_t& t, double capacity, mc_graph_t& graph, bool create_reverse_edge = false )
{
  auto capacitymap = get( boost::edge_capacity, graph );
  auto edgeinfomap = get( boost::edge_name,     graph );
  auto reversemap  = get( boost::edge_reverse,  graph );

  auto edge = boost::add_edge( s, t, graph ).first;

  edgeinfomap[edge].original_capacity = capacitymap[edge] = capacity;

  if ( create_reverse_edge )
  {
    auto redge = boost::add_edge( t, s, graph ).first;
    edgeinfomap[redge].original_capacity = capacitymap[redge] = 0.0;
    reversemap[edge] = redge;
    reversemap[redge] = edge;
  }

  return edge;
}

void add_reverse_edges( mc_graph_t& graph )
{
  auto capacitymap = get( boost::edge_capacity, graph );
  auto reversemap  = get( boost::edge_reverse,  graph );
  auto edgeinfomap = get( boost::edge_name,     graph );

  std::list<mc_edge_t> store_edges;
  boost::push_back( store_edges, boost::make_iterator_range( edges( graph ) ) );

  for ( const auto& edge : store_edges )
  {
    auto redge = boost::add_edge( boost::target( edge, graph ), boost::source( edge, graph ), graph ).first;
    edgeinfomap[redge].original_capacity = capacitymap[redge] = 0.0;
    reversemap[edge] = redge;
    reversemap[redge] = edge;
  }
}

std::pair<mc_vertex_t, mc_vertex_t> create_mincut_graph_with_splitting( mc_graph_t& graph, const aig_graph& aig )
{
  auto namemap  = get( boost::edge_name, graph   );
  auto vnamemap = get( boost::vertex_name, graph );

  /* A map to store AIG node to MC graph node */
  std::map<aig_node, std::pair<mc_vertex_t, mc_vertex_t>> node_map;

  /* Source and target */
  mc_vertex_t source = boost::add_vertex( graph );
  mc_vertex_t target = boost::add_vertex( graph );

  vnamemap[source].type = mc_source;
  vnamemap[target].type = mc_target;

  /* Copy nodes */
  for ( const aig_node& node : boost::make_iterator_range( boost::vertices( aig ) ) )
  {
    mc_vertex_t s = boost::add_vertex( graph );
    mc_vertex_t t = boost::add_vertex( graph );
    vnamemap[s].original_node = node;
    vnamemap[t].original_node = node;

    mc_edge_t e = add_edge( s, t, 1.0, graph );
    namemap[e].original_node = node;

    node_map[node] = std::make_pair( s, t );
  }

  /* Copy edges */
  for ( const aig_edge& edge : boost::make_iterator_range( boost::edges( aig ) ) )
  {
    const mc_vertex_t& s = node_map[boost::source( edge, aig )].second;
    const mc_vertex_t& t = node_map[boost::target( edge, aig )].first;

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

  return std::make_pair( source, target );
}

struct find_mincut_splitting_dump_dot_writer
{
  find_mincut_splitting_dump_dot_writer( const mc_graph_t& graph ) : graph( graph ) {}

  void operator()( std::ostream& os, const mc_vertex_t& node )
  {
    auto info = get( boost::vertex_name, graph )[node];

    if ( info.type == mc_source )
    {
      os << boost::format( "[label=\"s (%d)\",shape=box]" ) % node;
    }
    else if ( info.type == mc_target )
    {
      os << boost::format( "[label=\"t (%d)\",shape=box]" ) % node;
    }
    else
    {
      assert( info.original_node );
      os << "[label=\"" << ( 2u * *info.original_node ) << "\"]";
    }
  }

  void operator()( std::ostream& os, const mc_edge_t& edge )
  {
    auto capacity = get( boost::edge_capacity, graph );

    if ( capacity[edge] == 0u )
    {
      os << "[color=red,style=dotted]";
    }
    else
    {
      os << "[label=\"" << capacity[edge] << "\"]";
    }
  }

private:
  const mc_graph_t& graph;
};

void find_mincut_splitting_dump_dot( const mc_graph_t& graph, const std::string& filename )
{
  std::filebuf fb;
  fb.open( filename.c_str(), std::ios::out );
  std::ostream os( &fb );
  find_mincut_splitting_dump_dot_writer writer( graph );
  write_graphviz( os, graph, writer, writer );
  fb.close();
}

class has_one_weight_edges_visitor : public boost::default_dfs_visitor
{
public:
  has_one_weight_edges_visitor( bool& found ) : found( found )
  {
    found = false;
  }

  template<typename Edge>
  void examine_edge( Edge e, const mc_graph_t& graph )
  {
    if ( get( boost::edge_capacity, graph )[e] == 1.0 )
    {
      found = true;
    }
  }

  bool& found;
};

bool has_one_weight_edges( const mc_graph_t& graph, const mc_vertex_t& source )
{
  bool found;
  std::map<mc_vertex_t, boost::default_color_type> colors;
  boost::depth_first_visit( graph, source, has_one_weight_edges_visitor( found ), boost::make_assoc_property_map( colors ) );
  return found;
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

bool find_mincut_splitting( std::list<std::list<aig_node>>& cuts, aig_graph& aig, unsigned count, properties::ptr settings, properties::ptr statistics )
{
  /* settings */
  bool        verbose = get( settings, "verbose", false         );
  std::string dotname = get( settings, "dotname", std::string() );

  /* this keeps track of next source and target nodes */
  std::deque<std::pair<mc_vertex_t, mc_vertex_t>> sts;

  /* construct initial graph */
  mc_graph_t graph;
  mc_vertex_t source, target;

  std::tie( source, target ) = create_mincut_graph_with_splitting( graph, aig );
  sts.push_back({source, target});

  /* find cuts */
  for ( unsigned i = 0u; i < count; ++i )
  {
    assert( !sts.empty() );

    if ( verbose )
    {
      std::cout << "[I] find min cut " << i << std::endl;
    }

    if ( !dotname.empty() )
    {
      std::string filename = boost::str( boost::format( dotname ) % i );
      if ( verbose )
      {
        std::cout << "[I] write cut graph to " << filename << std::endl;
      }
      find_mincut_splitting_dump_dot( graph, filename );
    }

    std::tie( source, target ) = sts.front();
    sts.pop_front();

    if ( verbose )
    {
      std::cout << boost::format( "[I] perform max-flow computation with s = %d and t = %d" ) % source % target << std::endl;
    }
    boykov_kolmogorov_max_flow( graph, source, target );

    auto capacity = boost::get( boost::edge_capacity, graph );
    auto color    = boost::get( boost::vertex_color,  graph );
    auto name     = boost::get( boost::edge_name,     graph );
    auto vname    = boost::get( boost::vertex_name,   graph );

    std::list<aig_node> cut;

    mc_vertex_t new_target = boost::add_vertex( graph );
    mc_vertex_t new_source = boost::add_vertex( graph );
    vname[new_target].type = mc_target;
    vname[new_source].type = mc_source;

    std::list<std::pair<mc_vertex_t, mc_vertex_t>> st_vertices;

    for ( const auto& e : boost::make_iterator_range( edges( graph ) ) )
    {
      if ( capacity[e] > 0 )
      {
        if ( color[boost::source(e, graph)] != color[boost::target(e, graph)] )
        {
          if ( name[e].original_node )
          {
            cut += *( name[e].original_node );
          }
          else
          {
            if ( verbose )
            {
              std::cout << "[W] no node assigned to " << e << std::endl;
            }
          }

          /* New source and target */
          st_vertices += std::make_pair( boost::source( e, graph ), boost::target( e, graph ) );
        }
      }

      capacity[e] = name[e].original_capacity;
    }

    for ( const auto& p : st_vertices )
    {
      mc_vertex_t s, t;
      std::tie( s, t ) = p;
      add_edge( s, new_target, std::numeric_limits<double>::infinity(), graph, true );
      add_edge( new_source, t, std::numeric_limits<double>::infinity(), graph, true );
      remove_edge( s, t, graph );
      remove_edge( t, s, graph );
    }

    if ( has_one_weight_edges( graph, source ) )
    {
      sts.push_back({source, new_target});
    }
    if ( has_one_weight_edges( graph, new_source ) )
    {
      sts.push_back({new_source, target});
    }

    cuts += cut;
  }

  return true;
}

mincut_by_node_func find_mincut_splitting_func( properties::ptr settings, properties::ptr statistics )
{
  mincut_by_node_func f = [&settings, &statistics]( std::list<std::list<aig_node>>& cuts, aig_graph& aig, unsigned count ) {
    return find_mincut_splitting( cuts, aig, count, settings, statistics );
  };
  f.init( settings, statistics );
  return f;
}

}

// Local Variables:
// c-basic-offset: 2
// End:
