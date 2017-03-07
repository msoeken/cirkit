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
 * @file depth.hpp
 *
 * @brief Computes the depth of a DAG
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef DEPTH_HPP
#define DEPTH_HPP

#include <map>
#include <vector>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/range/iterator_range.hpp>

#include <core/utils/graph_utils.hpp>

namespace cirkit
{

template<class Graph>
class depth_visitor : public boost::default_dfs_visitor
{
public:
  depth_visitor( std::vector<unsigned>& depths, unsigned& max_depth )
    : depths( depths ),
      max_depth( max_depth ) {}

  virtual void finish_vertex( const vertex_t<Graph>& node, const Graph& g )
  {
    if ( boost::out_degree( node, g ) == 0u )
    {
      depths[node] = 0u;
    }
    else
    {
      auto d = 0u;
      for ( const auto& a : boost::make_iterator_range( boost::adjacent_vertices( node, g ) ) )
      {
        d = std::max( d, depths[a] + 1u );
      }

      depths[node] = d;
      if ( d > max_depth ) { max_depth = d; }
    }
  }

private:
  std::vector<unsigned>& depths;
  unsigned&              max_depth;
};

template<class Graph>
unsigned compute_depth( const Graph& g, const std::vector<vertex_t<Graph>>& outputs, std::vector<unsigned>& depths )
{
  std::map<vertex_t<Graph>, boost::default_color_type> colors;

  depths.resize( boost::num_vertices( g ), 0u );

  auto max_depth = 0u;
  depth_visitor<Graph> visitor( depths, max_depth );

  for ( const auto& node : outputs )
  {
    boost::depth_first_visit( g, node, visitor, boost::make_assoc_property_map( colors ) );
  }

  return max_depth;
}

template<class Graph>
unsigned compute_depth( const Graph& g, std::vector<unsigned>& depths )
{
  std::vector<vertex_t<Graph>> outputs;

  /* pre-compute indegrees */
  auto indegrees = precompute_in_degrees( g );

  /* compute outputs if not provided */
  if ( outputs.empty() )
  {
    for ( const auto& v : boost::make_iterator_range( boost::vertices( g ) ) )
    {
      if ( !indegrees[v] )
      {
        outputs += v;
      }
    }
  }

  return compute_depth( g, outputs, depths );
}

template<class Graph>
unsigned compute_depth( const Graph& g )
{
  std::vector<unsigned> depths;

  return compute_depth( g, depths );
}

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
