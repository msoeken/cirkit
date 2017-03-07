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
