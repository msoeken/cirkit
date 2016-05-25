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

#include "mig_dfs.hpp"

#include <boost/range/algorithm.hpp>

namespace cirkit
{

/******************************************************************************
 * mig_dfs_visitor                                                            *
 ******************************************************************************/

mig_dfs_visitor::mig_dfs_visitor( const mig_graph& mig )
  : boost::default_dfs_visitor(),
    graph_info( boost::get_property( mig, boost::graph_name ) )
{
}

void mig_dfs_visitor::finish_vertex( const mig_node& node, const mig_graph& mig )
{
  boost::default_dfs_visitor::finish_vertex( node, mig );
  if ( boost::find( graph_info.inputs, node ) != graph_info.inputs.end() )
  {
    const auto& it = graph_info.node_names.find( node );
    if ( it != graph_info.node_names.end() )
    {
      finish_input( node, it->second, mig );
    }
    else
    {
      finish_input( node, "", mig );
    }
  }
  else if ( node == graph_info.constant )
  {
    finish_constant( node, mig );
  }
  else
  {
    const auto& complement_map = boost::get( boost::edge_complement, mig );
    assert( out_degree( node, mig ) == 3u );
    auto itEdge = out_edges( node, mig ).first;
    const auto& a = *  itEdge;
    const auto& b = *( itEdge + 1 );
    const auto& c = *( itEdge + 2 );
    finish_mig_node( node, {boost::target( a, mig ), complement_map[a]},
                           {boost::target( b, mig ), complement_map[b]},
                           {boost::target( c, mig ), complement_map[c]}, mig );
  }
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
