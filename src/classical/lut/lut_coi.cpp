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

#include "lut_coi.hpp"

#include <boost/dynamic_bitset.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/topological_sort.hpp>

#include <iostream>
#include <map>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

void lut_compute_coi_rec( const lut_graph& graph, const lut_vertex_t& node, std::vector<lut_vertex_t>* primary_inputs, unsigned& count )
{
  if ( graph.is_marked( node ) )
  {
    return;
  }

  graph.mark( node );
  ++count;

  if ( graph.is_input( node ) )
  {
    if ( node != 0 && node != 1 && primary_inputs )
    {
      primary_inputs->push_back( node );
    }
  }
  else
  {
    for ( const auto& c : graph.children( node ) )
    {
      lut_compute_coi_rec( graph, c, primary_inputs, count );
    }
  }
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

unsigned lut_compute_coi( const lut_graph& graph, const lut_vertex_t& start_node, std::vector<lut_vertex_t>* primary_inputs )
{
  graph.init_marks();
  auto count = 0u;

  lut_compute_coi_rec( graph, start_node, primary_inputs, count );

  return count;
}

boost::dynamic_bitset<> lut_compute_coi_as_bitset( const lut_graph& graph, const lut_vertex_t& start_node )
{
  boost::dynamic_bitset<> cone( graph.nodes().size() - /* constants */ 2u, 0u );

  graph.init_marks();
  auto count = 0u;

  lut_compute_coi_rec( graph, start_node, nullptr, count );

  for ( const auto& n : graph.nodes() )
  {
    if ( n == 0 || n == 1 ) continue;
    assert( n < cone.size()+2u );
    if ( graph.is_marked( n ) )
    {
      cone[n-2u] = true;
    }
  }

  assert( cone.count() == count );
  return cone;
}

std::vector<lut_vertex_t> lut_coi_topological_nodes( const lut_graph& graph )
{
  boost::dynamic_bitset<> mark( graph.size() );
  std::vector<std::pair<lut_vertex_t, unsigned>> output_cois;

  for ( const auto& output : graph.outputs() )
  {
    const auto node = output.first;

    if ( mark[node] ) { continue; }
    mark.set( node );

    output_cois.push_back( {node, lut_compute_coi( graph, node )} );
  }

  std::stable_sort( output_cois.begin(), output_cois.end(),
                    []( const std::pair<lut_vertex_t, unsigned>& p1, const std::pair<lut_vertex_t, unsigned>& p2 ) {
                      return p1.second > p2.second;
                    } );

  /* for DFS */
  graph.init_marks();
  std::vector<lut_vertex_t> result;

  for ( const auto& p : output_cois )
  {
    using vec_t = std::vector<lut_vertex_t>;
    using map_t = std::map<lut_vertex_t, boost::default_color_type>;
    using iterator = std::back_insert_iterator<vec_t>;
    map_t color_map;

    vec_t nodes;
    depth_first_visit( graph.graph(), p.first,
                       boost::topo_sort_visitor<iterator>( std::back_inserter( nodes ) ),
                       make_assoc_property_map( color_map ) );

    assert( nodes.size() == p.second );

    for ( const auto& n : nodes )
    {
      if ( graph.is_marked( n ) ) { continue; }
      graph.mark( n );

      result.push_back( n );
    }
  }

  return result;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
