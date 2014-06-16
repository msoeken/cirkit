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

#include "find_mincut.hpp"

#include <boost/assign/std/list.hpp>
#include <boost/graph/boykov_kolmogorov_max_flow.hpp>
#include <boost/range/iterator_range.hpp>

using namespace boost::assign;

namespace cirkit
{

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

void add_reverse_edge( aig_graph& aig, const aig_edge& edge, double capacity = 1.0 )
{
  auto capacitymap   = get( boost::edge_capacity,   aig );
  auto reversemap    = get( boost::edge_reverse,    aig );

  auto redge = add_edge( boost::target( edge, aig ), boost::source( edge, aig ), aig ).first;

  capacitymap[edge] = capacity;
  capacitymap[redge] = 0.0;
  reversemap[edge] = redge;
  reversemap[redge] = edge;
}

aig_edge add_reverse_edge( aig_graph& aig, const aig_node& from, const aig_node& to, double capacity = std::numeric_limits<double>::infinity() )
{
  auto capacitymap   = get( boost::edge_capacity,   aig );
  auto reversemap    = get( boost::edge_reverse,    aig );

  auto edge = add_edge( from, to, aig ).first;
  auto redge = add_edge( to, from, aig ).first;

  capacitymap[edge] = capacity;
  capacitymap[redge] = 0.0;
  reversemap[edge] = redge;
  reversemap[redge] = edge;

  return edge;
}

void prepare_graph( aig_graph& aig, aig_node& source, aig_node& target, std::list<aig_edge>& original_edges )
{
  const auto& graph_info = boost::get_property( aig, boost::graph_name );
  auto complementmap = get( boost::edge_complement, aig );
  auto capacitymap   = get( boost::edge_capacity,   aig );
  auto reversemap    = get( boost::edge_reverse,    aig );

  /* reverse edges */
  for ( const auto& edge : boost::make_iterator_range( edges( aig ) ) )
  {
    add_reverse_edge( aig, edge );
    original_edges += edge;
  }

  /* source and target */
  source = add_vertex( aig );
  target = add_vertex( aig );

  for ( const auto& input : graph_info.inputs )
  {
    add_reverse_edge( aig, input, target );
  }

  for ( const auto& output : graph_info.outputs )
  {
    auto output_node = add_vertex( aig );
    auto edge = add_reverse_edge( aig, output_node, output.first.first, 1.0 );
    complementmap[edge] = output.first.second;
    original_edges += edge;

    add_reverse_edge( aig, source, output_node );
  }

  /* constant */
  add_reverse_edge( aig, graph_info.constant, target );
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

void find_mincut( aig_graph& aig, unsigned count, std::list<std::list<aig_function>>& cuts )
{
  aig_node source, target;
  std::list<aig_edge> original_edges;
  std::list<aig_edge> cut_edges;

  prepare_graph( aig, source, target, original_edges );

  for ( unsigned i = 0u; i < count; ++i )
  {
    boykov_kolmogorov_max_flow( aig, source, target );

    auto capacity   = boost::get( boost::edge_capacity,   aig );
    auto complement = boost::get( boost::edge_complement, aig );
    auto color      = boost::get( boost::vertex_color,    aig );

    std::list<aig_function> cut;

    for ( const auto& e : boost::make_iterator_range( edges( aig ) ) )
    {
      if ( capacity[e] > 0 )
      {
        if ( color[boost::source(e, aig)] != color[boost::target(e, aig)] )
        {
          cut += std::make_pair( boost::target(e, aig), complement[e] );
          cut_edges += e;
        }
      }
    }

    for ( const auto& e : original_edges )
    {
      capacity[e] = 1.0;
    }

    for ( const auto& e : cut_edges )
    {
      capacity[e] = std::numeric_limits<double>::infinity();
    }

    cuts += cut;
  }
}

}

// Local Variables:
// c-basic-offset: 2
// End:
