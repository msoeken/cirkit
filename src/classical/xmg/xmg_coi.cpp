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

#include "xmg_coi.hpp"

#include <iostream>
#include <map>

#include <boost/dynamic_bitset.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/topological_sort.hpp>

#include <classical/xmg/xmg_bitmarks.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

void xmg_compute_coi_rec( xmg_graph& xmg, xmg_node node, std::vector<xmg_node>* primary_inputs, unsigned& count )
{
  if ( xmg.bitmarks().is_marked( node ) )
  {
    return;
  }

  xmg.bitmarks().mark( node );
  ++count;

  if ( xmg.is_input( node ) )
  {
    if ( node != 0 && primary_inputs )
    {
      primary_inputs->push_back( node );
    }
  }
  else
  {
    for ( const auto& c : xmg.children( node ) )
    {
      xmg_compute_coi_rec( xmg, c.node, primary_inputs, count );
    }
  }
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

unsigned xmg_compute_coi( xmg_graph& xmg, xmg_node start_node, std::vector<xmg_node>* primary_inputs )
{
  xmg.bitmarks().init_marks( xmg.size() );
  auto count = 0u;

  xmg_compute_coi_rec( xmg, start_node, primary_inputs, count );

  return count;
}

std::vector<xmg_node> xmg_coi_topological_nodes( xmg_graph& xmg )
{
  boost::dynamic_bitset<> mark( xmg.size() );
  std::vector<std::pair<xmg_node, unsigned>> output_cois;

  for ( const auto& output : xmg.outputs() )
  {
    const auto node = output.first.node;

    if ( mark[node] ) { continue; }
    mark.set( node );

    output_cois.push_back( {node, xmg_compute_coi( xmg, node )} );
  }

  std::stable_sort( output_cois.begin(), output_cois.end(),
                    []( const std::pair<xmg_node, unsigned>& p1, const std::pair<xmg_node, unsigned>& p2 ) {
                      return p1.second > p2.second;
                    } );

  /* for DFS */
  xmg.bitmarks().init_marks( xmg.size() );
  std::vector<xmg_node> result;

  for ( const auto& p : output_cois )
  {
    using vec_t = std::vector<xmg_node>;
    using map_t = std::map<xmg_node, boost::default_color_type>;
    using iterator = std::back_insert_iterator<vec_t>;
    map_t color_map;

    vec_t nodes;
    depth_first_visit( xmg.graph(), p.first,
                       boost::topo_sort_visitor<iterator>( std::back_inserter( nodes ) ),
                       make_assoc_property_map( color_map ) );

    assert( nodes.size() == p.second );

    for ( const auto& n : nodes )
    {
      if ( xmg.bitmarks().is_marked( n ) ) { continue; }
      xmg.bitmarks().mark( n );

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
