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
#include <classical/graphviz.hpp>
#include <boost/format.hpp>

namespace cirkit
{

std::string port_string( const aig_graph& aig, const aig_node& n, const std::string& port )
{
  const unsigned lit = aig_to_literal( aig, n );
  if ( out_degree( n, aig ) == 2 )
  {
    return ( boost::format("gt%s.%s") % lit % port ).str();
  }
  else
  {
    return ( boost::format("gt%s.north") % lit ).str();
  }
}

void write_tex( const aig_graph& aig, const std::string& layout_algorithm, std::ostream& os, const bool fill_sym_table )
{
  assert( num_vertices( aig ) != 0u && "Uninitialized AIG" );

  gv_graph gv;
  gv_initialize( gv, "AIG" );

  std::map< aig_node, gv_node > node_map;
  std::map< aig_edge, gv_edge > edge_map;

  compute_graphviz_layout( gv, aig, layout_algorithm, "dot", node_map, edge_map );

  /* generate tex */
  os << "\\begin{tikzpicture}[>=latex',circuit logic US]\n";
  os << "\\tikzstyle{and} = [draw,and port,rotate=90];\n";
  os << "\\tikzstyle{not} = [draw,thick,fill=white];\n";

  /* extract layout information from each node */
  for ( const auto& node : boost::make_iterator_range( vertices( aig ) ) )
  {
    const unsigned lit = aig_to_literal( aig, node );
    const auto& n = node_map.find( node );
    const gv_node& gv_n = (node_map.find( node ))->second;
    const unsigned x = gv_coord_x( gv_n );
    const unsigned y = gv_coord_y( gv_n );

    /* write gate shape */
    if ( out_degree( node, aig ) == 2 ) {
    os << ( boost::format("\\node[draw,and] (gt%s) at (%sbp,%sbp) {\\tiny %s};\n") % lit % x % y % lit );
    }
    else
    {
      os << ( boost::format("\\node (gt%s) at (%sbp,%sbp) {\\tiny %s};\n") % lit % x % y % lit );
    }
  }

  /* extract layout information from each node */
  for ( const auto& node : boost::make_iterator_range( vertices( aig ) ) )
  {
    const auto _out_degree = out_degree( node, aig );
    if ( _out_degree )
    {
      assert( _out_degree == 2u );

      const auto &edges = out_edges( node, aig );
      assert( std::distance(edges.first,edges.second) == 2 );

      /* left edge */
      auto edge_it = edges.first;
      {
        assert( edge_it != edges.second );
        const aig_edge& le = *edge_it++;

        std::string style;
        if ( source( le, aig ) % 2 != 0 )
        {
          style = "dashed";
        }

        const gv_edge& gv_e = edge_map.find( le )->second;

        const std::string s = port_string( aig, target( le, aig ), "out" );
        const std::string t = port_string( aig, source( le, aig ), "in 1" );

        os << ( boost::format("\\draw (%s) |- ($(%s)!.4!(%s)$) -| (%s);\n") % s % s % t % t );

        if ( source( le, aig ) % 2 != 0 )
        {
          os << ( boost::format("\\filldraw[not] ([yshift=3pt] %s) circle (3pt);\n") % t );
        }
      }

      /* right edge */
      {
        assert( edge_it != edges.second );
        const aig_edge& ri = *edge_it++;
        const gv_edge& gv_e = edge_map.find( ri )->second;

        const std::string s = port_string( aig, target( ri, aig ), "out" );
        const std::string t = port_string( aig, source( ri, aig ), "in 2" );

        os << ( boost::format("\\draw (%s) |- ($(%s)!.6!(%s)$) -| (%s);\n") % s % s % t % t );

        if ( source( ri, aig ) % 2 != 0 )
        {
          os << ( boost::format("\\filldraw[not] ([yshift=3pt] %s) circle (3pt);\n") % t );
        }
      }
    }
  }

  const auto& graph_info = boost::get_property( aig, boost::graph_name );

  /* input names */
  unsigned index = 0u;
  for ( const auto& input : graph_info.inputs )
  {
    auto it = graph_info.node_names.find( input );
    const unsigned lit = aig_to_literal( aig, input );
    if ( it != graph_info.node_names.end() )
    {
      os << ( boost::format("\\node[right=0.01cm,yshift=4pt] at (gt%s.east) {\\tiny i%s %s};\n") % lit % index % it->second );
    }
    else if ( fill_sym_table )
    {
      os << ( boost::format("\\node[right=0.01cm,yshift=4pt] at (gt%s.east) {\\tiny i%s input%s};\n") % lit % index % index );
    }
    ++index;
  }

  /* output names */
  index = 0u;
  for ( const auto& output : graph_info.outputs )
  {
    const std::string& name = output.second;
    const unsigned lit = aig_to_literal( aig, output.first );
    if ( name != "" )
    {
      os << ( boost::format("\\node[right=0.01cm,yshift=4pt] at (gt%s.out) {\\tiny o%s %s};\n") % lit % index % name );
    }
    else if ( fill_sym_table )
    {
      os << ( boost::format("\\node[right=0.01cm,yshift=4pt] at (gt%s.out) {\\tiny o%s output%s};\n") % lit % index % index );
    }
    ++index;
  }

  os << "\\end{tikzpicture}\n";
}

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
