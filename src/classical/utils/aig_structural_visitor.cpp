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

/**
 * @author Heinz Riener
 */

#include "aig_structural_visitor.hpp"

#include <classical/utils/aig_utils.hpp>

namespace cirkit
{

void aig_visit_nodes( const aig_graph& aig, const aig_structural_visitor& vis )
{
  const auto& info = aig_info( aig );
  const auto& complementmap = boost::get( boost::edge_complement, aig );

  if ( aig_is_constant_used( aig ) )
  {
    vis.visit_constant( info.constant );
  }

  /* iterate through nodes in order */
  for ( const auto& node : boost::make_iterator_range( vertices( aig ) ) )
  {
    const auto _out_degree = out_degree( node, aig );

    /* input */
    if ( boost::find( info.inputs, node ) != info.inputs.end() )
    {
      vis.visit_input( node );
    }

    /* latches */
    if ( _out_degree == 1u )
    {
      vis.visit_latch( node );
    }
    /* and gates */
    else if ( _out_degree == 2u )
    {
      const auto& edges = boost::make_iterator_range( out_edges( node, aig ) );
      assert( edges.size() == 2u );
      aig_function l = { boost::target( edges[ 0u ], aig), complementmap[ edges[ 0u ] ] };
      aig_function r = { boost::target( edges[ 1u ], aig), complementmap[ edges[ 1u ] ] };
      vis.visit_and( node, l, r );
    }

    /* output */
    for ( const auto &o : info.outputs )
    {
      if ( o.first.node == node )
      {
        vis.visit_output( node );
      }
    }
  }
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
