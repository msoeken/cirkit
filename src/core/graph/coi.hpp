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
 * @file coi.hpp
 *
 * @brief Cone-of-influence functions
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef CIRKIT_COI_HPP
#define CIRKIT_COI_HPP

#include <stack>
#include <unordered_set>

#include <boost/graph/adjacency_list.hpp>
#include <boost/range/iterator_range.hpp>

#include <core/utils/graph_utils.hpp>

namespace cirkit
{

template<class Graph>
std::unordered_set<vertex_t<Graph>> compute_coi( const Graph& g, const vertex_t<Graph>& node )
{
  std::unordered_set<vertex_t<Graph>> coi;

  std::stack<vertex_t<Graph>> stack;
  stack.push( node );

  while ( !stack.empty() )
  {
    auto top = stack.top(); stack.pop();

    if ( coi.find( top ) != coi.end() ) { continue; }

    coi.insert( top );

    for ( const auto& child : boost::make_iterator_range( boost::adjacent_vertices( top, g ) ) )
    {
      stack.push( child );
    }
  }

  return std::move( coi );
}

template<class Graph>
unsigned compute_coi_size( const Graph& g, const vertex_t<Graph>& node )
{
  return compute_coi( g, node ).size();
}

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
