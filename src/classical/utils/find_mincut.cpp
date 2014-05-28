/* RevKit: A Toolkit for Reversible Circuit Design (www.revkit.org)
 * Copyright (C) 2009-2013  The RevKit Developers <revkit@informatik.uni-bremen.de>
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

#include "find_mincut.hpp"

#include <boost/assign/std/list.hpp>
#include <boost/graph/boykov_kolmogorov_max_flow.hpp>
#include <boost/range/iterator_range.hpp>

namespace revkit
{

void find_mincut( aig_graph& graph, std::list<unsigned>& cut )
{
  using namespace boost::assign;

  boykov_kolmogorov_max_flow( graph, 0, 1 );

  auto capacity = boost::get( boost::edge_capacity, graph );
  auto name     = boost::get( boost::vertex_name,   graph );
  auto color    = boost::get( boost::vertex_color,  graph );

  for ( const auto& e : boost::make_iterator_range( edges( graph ) ) )
  {
    if ( capacity[e] > 0 )
    {
      if ( color[source(e, graph)] != color[target(e, graph)] )
      {
        cut += name[source(e, graph)];
      }
    }
  }
}

}

// Local Variables:
// c-basic-offset: 2
// End:
