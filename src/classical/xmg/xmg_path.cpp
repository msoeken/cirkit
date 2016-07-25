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
