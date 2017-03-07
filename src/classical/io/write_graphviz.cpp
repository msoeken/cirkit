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

#if ADDON_GRAPHVIZ

#include <classical/io/write_graphviz.hpp>
#include <boost/format.hpp>

namespace cirkit
{

void compute_graphviz_layout( gv_graph& gv, const aig_graph& aig, const std::string& layout_algorithm, const std::string& render_format,
                              std::map< aig_node, gv_node >& node_map, std::map< aig_edge, gv_edge >& edge_map )
{

  /* nodes */
  for ( const auto& node : boost::make_iterator_range( vertices( aig ) ) )
  {
    const std::string name = ( boost::format("%s") % aig_to_literal(aig,node) ).str();
    node_map.insert( {node, gv_add_node(gv, name)} );
  }

  /* edges */
  for ( const auto& node : boost::make_iterator_range( vertices( aig ) ) )
  {
    const auto &src = node_map.find( node );
    if ( src == node_map.end() ) throw "Source of edge undefined";

    for ( const auto& edge : boost::make_iterator_range( out_edges( node, aig ) ) )
    {
      const auto &dst = node_map.find( target( edge, aig ) );
      if ( dst == node_map.end() ) throw "Target of edge undefined";

      edge_map.insert( {edge,gv_add_edge( gv, src->second, dst->second )} );
    }
  }

  gv_layout( gv, layout_algorithm );
}

void write_graphviz( const aig_graph& aig, const std::string& layout_algorithm, const std::string& render_format, std::ostream& os )
{
  std::map< aig_node, gv_node > node_map;
  std::map< aig_edge, gv_edge > edge_map;
  write_graphviz( aig, layout_algorithm, render_format, node_map, edge_map, os );
}

void write_graphviz( const aig_graph& aig, const std::string& layout_algorithm, const std::string& render_format,
                     std::map< aig_node, gv_node >& node_map, std::map< aig_edge, gv_edge >& edge_map, std::ostream& os )
{
  assert( num_vertices( aig ) != 0u && "Uninitialized AIG" );

  gv_graph gv;
  gv_initialize( gv, "AIG" );

  compute_graphviz_layout( gv, aig, layout_algorithm, render_format, node_map, edge_map );
  gv_render( gv, render_format, os );
}

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
