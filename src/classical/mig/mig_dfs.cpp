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
