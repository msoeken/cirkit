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
 * @file coi.hpp
 *
 * @brief Cone-of-influence functions
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef CIRKIT_COI_HPP
#define CIRKIT_COI_HPP

#include <algorithm>
#include <stack>
#include <vector>

#include <boost/dynamic_bitset.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/range/iterator_range.hpp>

#include <core/utils/graph_utils.hpp>

namespace cirkit
{

template<class Graph>
std::vector<vertex_t<Graph>> compute_coi( const Graph& g, const vertex_t<Graph>& node )
{
  std::vector<vertex_t<Graph>> coi; /* in reverse topological order */
  boost::dynamic_bitset<> mark( boost::num_vertices( g ) );

  std::stack<vertex_t<Graph>> stack;
  stack.push( node );

  while ( !stack.empty() )
  {
    auto top = stack.top(); stack.pop();

    if ( mark[top] ) { continue; }
    coi.push_back( top );
    mark.set( top );

    for ( const auto& child : boost::make_iterator_range( boost::adjacent_vertices( top, g ) ) )
    {
      stack.push( child );
    }
  }

  return coi;
}

template<class Graph>
unsigned compute_coi_size( const Graph& g, const vertex_t<Graph>& node )
{
  return compute_coi( g, node ).size();
}

template<class Graph>
std::vector<vertex_t<Graph>> coi_topological_nodes( const Graph& g, const std::vector<vertex_t<Graph>>& nodes )
{
  /* get cois for each output */
  using pair_t = std::pair<vertex_t<Graph>, std::vector<vertex_t<Graph>>>;
  std::vector<pair_t> output_cois;
  for ( const auto& n : nodes )
  {
    output_cois.emplace_back( n, compute_coi( g, n ) );
  }

  /* sort cois by size */
  std::stable_sort( output_cois.begin(), output_cois.end(),
                    []( const pair_t& p1, const pair_t& p2 ) {
                      return p1.second.size() > p2.second.size();
                    } );

  std::vector<vertex_t<Graph>> topo;
  boost::dynamic_bitset<> mark( boost::num_vertices( g ) );
  for ( const auto& p : output_cois )
  {
    for ( auto it = p.second.rbegin(); it != p.second.rend(); ++it )
    {
      if ( mark[*it] ) { continue; }
      topo.push_back( *it );
      mark.set( *it );
    }
  }

  return topo;
}

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
