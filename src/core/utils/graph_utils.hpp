/* RevKit (www.revkit.org)
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

#include <boost/assign/std/vector.hpp>
#include <boost/graph/adjacency_list.hpp>

using namespace boost::assign;

namespace cirkit
{

template<class VertexProperty = boost::no_property,
         class EdgeProperty = boost::no_property,
         class GraphProperty = boost::no_property>
using digraph_t = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, VertexProperty, EdgeProperty, GraphProperty>;

template<class G>
using vertex_t = typename boost::graph_traits<G>::vertex_descriptor;

template<class G>
using edge_t = typename boost::graph_traits<G>::edge_descriptor;

template<class G>
inline std::map<typename boost::graph_traits<G>::vertex_descriptor,
                std::vector<typename boost::graph_traits<G>::edge_descriptor>> precompute_ingoing_edges( const G& g )
{
  std::map<typename boost::graph_traits<G>::vertex_descriptor, std::vector<typename boost::graph_traits<G>::edge_descriptor>> m;
  for ( const auto& e : boost::make_iterator_range( boost::edges( g ) ) )
  {
    m[boost::target( e, g )] += e;
  }
  return m;
}


}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
