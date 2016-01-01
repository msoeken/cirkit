/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2016  EPFL
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

#include "aig_dfs.hpp"

#include <boost/range/algorithm.hpp>
#include <boost/range/iterator_range.hpp>

namespace cirkit
{

/******************************************************************************
 * aig_dfs_visitor                                                            *
 ******************************************************************************/

aig_dfs_visitor::aig_dfs_visitor( const aig_graph& aig )
  : boost::default_dfs_visitor(),
    graph_info( boost::get_property( aig, boost::graph_name ) )
{
}

void aig_dfs_visitor::finish_vertex( const aig_node& node, const aig_graph& aig )
{
  boost::default_dfs_visitor::finish_vertex( node, aig );
  if ( boost::find( graph_info.inputs, node ) != graph_info.inputs.end() )
  {
    const auto& it = graph_info.node_names.find( node );
    if ( it != graph_info.node_names.end() )
    {
      finish_input( node, it->second, aig );
    }
    else
    {
      finish_input( node, "", aig );
    }
  }
  else if ( node == graph_info.constant )
  {
    finish_constant( node, aig );
  }
  else
  {
    const auto& complement_map = boost::get( boost::edge_complement, aig );
    assert( out_degree( node, aig ) == 2u );
    auto itEdge = out_edges( node, aig ).first;
    const auto& left  = *  itEdge;
    const auto& right = *( itEdge + 1 );
    finish_aig_node( node, {boost::target( left, aig ), complement_map[left]}, {boost::target( right, aig ), complement_map[right]}, aig );
  }
}

/******************************************************************************
 * aig_partial_dfs                                                            *
 ******************************************************************************/

aig_partial_dfs::aig_partial_dfs( const aig_graph& aig, const term_func_opt& term )
  : _aig( aig ),
    _term( term )
{
  for ( const auto& v : boost::make_iterator_range( boost::vertices( _aig ) ) )
  {
    put( _color, v, color_type::white() );
  }
}

void aig_partial_dfs::search( const aig_node& node )
{
  boost::dfs_visitor<> vis;
  if ( (bool)_term )
  {
    boost::detail::depth_first_visit_impl( _aig, node, vis, _color, _term.get() );
  }
  else
  {
    boost::detail::depth_first_visit_impl( _aig, node, vis, _color, boost::detail::nontruth2() );
  }
}

aig_partial_dfs::color_amap& aig_partial_dfs::color()
{
  return _color;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
