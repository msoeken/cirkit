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

#include "xmg_dfs.hpp"

#include <boost/range/algorithm.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

xmg_dfs_visitor::xmg_dfs_visitor( const xmg_graph& xmg )
  : xmg( xmg )
{
}

void xmg_dfs_visitor::finish_vertex( const xmg_node& node, const xmg_graph::graph_t& g )
{
  boost::default_dfs_visitor::finish_vertex( node, g );
  if ( xmg.is_input( node ) )
  {
    if ( node == 0u )
    {
      finish_constant( node, xmg );
    }
    else
    {
      finish_input( node, xmg );
    }
  }
  else if ( xmg.is_xor( node ) )
  {
    const auto children = xmg.children( node );
    finish_xor_node( node, children[0u], children[1u], xmg );
  }
  else if ( xmg.is_maj( node ) )
  {
    const auto children = xmg.children( node );
    finish_maj_node( node, children[0u], children[1u], children[2u], xmg );
  }
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
