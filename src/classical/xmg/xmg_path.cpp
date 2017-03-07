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

#include "xmg_path.hpp"

#include <vector>

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

std::vector<std::vector<xmg_node>> xmg_find_critical_paths( xmg_graph& xmg )
{
  /* dynamic programming to compute each node delay */
  std::vector<std::pair<xmg_node, unsigned>> delays( xmg.size(), std::make_pair( 0, 0 ) );

  for ( auto node : xmg.topological_nodes() )
  {
    if ( xmg.is_input( node ) )
    {
      delays[node] = {node, 0u};
    }
    else
    {
      for ( auto child : xmg.children( node ) )
      {
        if ( delays[child.node].second + 1 >= delays[node].second )
        {
          delays[node].first = child.node;
          delays[node].second = delays[child.node].second + 1;
        }
      }
    }
  }

  std::vector<std::vector<xmg_node>> critical_paths;
  for ( const auto& output : xmg.outputs() )
  {
    std::vector<xmg_node> path;
    path.push_back( output.first.node );

    while ( delays[path.back()].first != path.back() )
    {
      path.push_back( delays[path.back()].first );
    }

    boost::reverse( path );

    critical_paths.push_back( path );
  }
  return critical_paths;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
