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

#include "aig_cone.hpp"

#include <boost/assign/std/vector.hpp>
#include <boost/format.hpp>
#include <boost/graph/copy.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/filtered_graph.hpp>
#include <boost/range/iterator_range.hpp>

#include <core/utils/graph_utils.hpp>
#include <core/utils/timer.hpp>
#include <classical/utils/aig_utils.hpp>

using namespace boost::assign;

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

using color_map_t     = std::map<aig_node, boost::default_color_type>;
using color_amap_t    = boost::associative_property_map<color_map_t>;
using color_value_t   = boost::property_traits<color_amap_t>::value_type;
using color_t         = boost::color_traits<color_value_t>;
using edge_filter_t   = edge_has_not_property<aig_graph, color_amap_t>;
using vertex_filter_t = vertex_has_not_property<color_amap_t>;
using filter_graph_t  = boost::filtered_graph<aig_graph, edge_filter_t, vertex_filter_t>;
using fg_vertex_t     = boost::graph_traits<filter_graph_t>::vertex_descriptor;
using index_map_t     = boost::property_map<filter_graph_t, boost::vertex_index_t>::type;
using iso_map_t       = boost::iterator_property_map<std::vector<aig_node>::iterator, index_map_t, fg_vertex_t, fg_vertex_t&>;


/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

template<class ColorMap>
void aig_cone_search_init( const aig_graph& aig, const ColorMap& color )
{
  for ( const auto& v : boost::make_iterator_range( boost::vertices( aig ) ) )
  {
    put( color, v, color_t::white() );
  }
}

/**
 * @brief A different implementation of the DFS search that does not
 *        re-initialize the vertex colors.
 */
template<class ColorMap>
void aig_cone_search( const aig_graph& aig, const ColorMap& color, const aig_node& start )
{
  boost::dfs_visitor<> vis;
  boost::detail::depth_first_visit_impl( aig, start, vis, color, boost::detail::nontruth2() );
}

aig_graph copy_from_filtered( const filter_graph_t& fg, std::vector<aig_node>& copy_map )
{
  aig_graph new_aig;
  copy_map.resize( boost::num_vertices( fg ) );
  iso_map_t copy_imap( copy_map.begin(), boost::get( boost::vertex_index, fg ) );
  boost::copy_graph( fg, new_aig, boost::orig_to_copy( copy_imap ) );
  return new_aig;
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

aig_graph aig_cone( const aig_graph& aig, const std::vector<std::string>& names,
                    const properties::ptr& settings,
                    const properties::ptr& statistics )
{
  /* settings */
  auto verbose = get( settings, "verbose", false );

  /* Timer */
  new_properties_timer t( statistics );

  const auto& info = aig_info( aig );

  /* depth first search */
  color_map_t color;
  auto acolor = make_assoc_property_map( color );
  aig_cone_search_init( aig, acolor );
  for ( const auto& name : names )
  {
    auto index = aig_output_index( info, name );
    if ( verbose )
    {
      std::cout << boost::format( "[i] starting dfs for output %s at index %d" ) % name % index << std::endl;
    }
    aig_cone_search( aig, acolor, info.outputs[index].first.first );
  }

  /* filter graph */
  filter_graph_t fg( aig, edge_filter_t( aig, acolor, color_t::white() ), vertex_filter_t( acolor, color_t::white() ) );

  /* copy graph */
  std::vector<aig_node> copy_map;
  aig_graph new_aig = copy_from_filtered( fg, copy_map );

  if ( verbose )
  {
    std::cout << boost::format( "[i] new graph has %d/%d vertices" ) % boost::num_vertices( new_aig ) % boost::num_vertices( aig ) << std::endl;
  }

  /* restore info */
  auto& new_info = aig_info( new_aig );

  /* restore PIs */
  for ( const auto& v : boost::make_iterator_range( boost::vertices( fg ) ) )
  {
    if ( boost::out_degree( v, fg ) == 0u )
    {
      if ( v == info.constant )
      {
        new_info.constant = copy_map[v];
        new_info.constant_used = true;

        if ( verbose ) { std::cout << "[i] constant is in cone" << std::endl; }
      }
      else
      {
        new_info.inputs += copy_map[v];
        new_info.node_names[copy_map[v]] = info.node_names.at( v );

        if ( verbose ) { std::cout << "[i] PI with name " << info.node_names.at( v ) << " is in cone" << std::endl; }
      }
    }
  }

  /* restore POs */
  for ( const auto& name : names )
  {
    const auto& output = info.outputs[aig_output_index( info, name )];
    new_info.outputs += std::make_pair( std::make_pair( copy_map[output.first.first], output.first.second ), name );
  }

  /* restore internal IDs */
  const auto& vertex_name = boost::get( boost::vertex_name, new_aig );
  for ( const auto& v : boost::make_iterator_range( boost::vertices( new_aig ) ) )
  {
    vertex_name[v] = v << 1u;
  }

  return new_aig;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
