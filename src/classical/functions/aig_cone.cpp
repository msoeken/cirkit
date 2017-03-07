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

#include "aig_cone.hpp"

#include <boost/assign/std/vector.hpp>
#include <boost/dynamic_bitset.hpp>
#include <boost/format.hpp>
#include <boost/graph/copy.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/filtered_graph.hpp>
#include <boost/range/iterator_range.hpp>

#include <core/utils/graph_utils.hpp>
#include <core/utils/range_utils.hpp>
#include <core/utils/timer.hpp>
#include <classical/functions/strash.hpp>
#include <classical/utils/aig_dfs.hpp>
#include <classical/utils/aig_utils.hpp>

using namespace boost::assign;

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

using color_amap_t    = aig_partial_dfs::color_amap;
using color_t         = aig_partial_dfs::color_type;
using edge_filter_t   = edge_has_not_property<aig_graph, color_amap_t>;
using vertex_filter_t = vertex_has_not_property<color_amap_t>;
using filter_graph_t  = boost::filtered_graph<aig_graph, edge_filter_t, vertex_filter_t>;
using fg_vertex_t     = boost::graph_traits<filter_graph_t>::vertex_descriptor;
using index_map_t     = boost::property_map<filter_graph_t, boost::vertex_index_t>::type;
using iso_map_t       = boost::iterator_property_map<std::vector<aig_node>::iterator, index_map_t, fg_vertex_t, fg_vertex_t&>;


/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

aig_graph aig_cone( const aig_graph& aig, const std::vector<std::string>& names,
                    const properties::ptr& settings,
                    const properties::ptr& statistics )
{
  std::vector<unsigned> indexes( names.size() );

  const auto& info = aig_info( aig );
  boost::transform( names, indexes.begin(), [&info]( const std::string& name ) { return aig_output_index( info, name ); } );

  return aig_cone( aig, indexes, settings, statistics );
}

aig_graph aig_cone( const aig_graph& aig, const std::vector<unsigned>& indexes,
                    const properties::ptr& settings,
                    const properties::ptr& statistics )
{
  /* settings */
  auto verbose = get( settings, "verbose", false );

  /* Timer */
  properties_timer t( statistics );

  const auto& info = aig_info( aig );

  /* depth first search */
  aig_partial_dfs dfs( aig );
  for ( auto index : indexes )
  {
    if ( verbose )
    {
      std::cout << boost::format( "[i] starting dfs for output at index %d" ) % index << std::endl;
    }
    dfs.search( info.outputs[index].first.node );
  }

  /* make sure that we copy the constant vertex */
  put( dfs.color(), info.constant, color_t::black() );

  /* filter graph */
  filter_graph_t fg( aig, edge_filter_t( aig, dfs.color(), color_t::white() ), vertex_filter_t( dfs.color(), color_t::white() ) );

  /* copy graph */
  std::vector<aig_node> copy_map;
  aig_graph new_aig = copy_from_filtered<aig_graph, filter_graph_t>( fg, copy_map );

  if ( verbose )
  {
    std::cout << boost::format( "[i] new graph has %d/%d vertices" ) % boost::num_vertices( new_aig ) % boost::num_vertices( aig ) << std::endl;
  }

  /* restore info */
  auto& new_info = aig_info( new_aig );
  new_info.constant_used = false;

  /* restore PIs */
  // for ( const auto& v : boost::make_iterator_range( boost::vertices( fg ) ) )
  // {
  //   if ( boost::out_degree( v, fg ) == 0u )
  //   {
  //     if ( v == info.constant )
  //     {
  //       new_info.constant = copy_map[v];
  //       new_info.constant_used = true;
  //       assert( new_info.constant == 0 );

  //       if ( verbose ) { std::cout << "[i] constant is in cone" << std::endl; }
  //     }
  //     else
  //     {
  //       new_info.inputs += copy_map[v];
  //       new_info.node_names[copy_map[v]] = info.node_names.at( v );

  //       if ( verbose ) { std::cout << "[i] PI with name " << info.node_names.at( v ) << " is in cone" << std::endl; }
  //     }
  //   }
  // }

  assert( copy_map[0u] == 0u );
  new_info.constant = 0u;
  new_info.constant_used = info.constant_used;

  boost::dynamic_bitset<> mapped_inputs( info.inputs.size() );
  for ( const auto& in : index( info.inputs ) )
  {
    if ( get( dfs.color(), in.value ) != color_t::white() )
    {
      mapped_inputs.set( in.index );
      new_info.inputs += copy_map[in.value];
      new_info.node_names[copy_map[in.value]] = info.node_names.at( in.value );
    }
  }
  set( statistics, "mapped_inputs", mapped_inputs );

  /* restore POs */
  for ( const auto& index : indexes )
  {
    const auto& output = info.outputs[index];
    const aig_function f = { copy_map[output.first.node], output.first.complemented };
    new_info.outputs += std::make_pair( f, output.second );
  }

  /* restore internal IDs */
  const auto& vertex_name = boost::get( boost::vertex_name, new_aig );
  for ( const auto& v : boost::make_iterator_range( boost::vertices( new_aig ) ) )
  {
    vertex_name[v] = v << 1u;
  }

  return strash( new_aig );
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
