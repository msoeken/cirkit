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

#include "aig.hpp"

#include <boost/assign/std/vector.hpp>
#include <boost/graph/graphviz.hpp>

namespace revkit
{

using namespace boost::assign;

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

void aig_initialize( aig_graph& aig )
{
  /* create constant node */
  aig_node node = add_vertex( aig );
  boost::get( boost::vertex_name, aig )[node] = 0u;
}

aig_node aig_create_pi( aig_graph& aig, const std::string& name )
{
  aig_node node = add_vertex( aig );
  boost::get( boost::vertex_name, aig )[node] = 2u * num_vertices( aig );

  boost::get_property( aig, boost::graph_name ).inputs += node;
  boost::get_property( aig, boost::graph_name ).node_names[node] = name;
}

void aig_create_po( aig_graph& aig, const aig_node& node, const std::string& name, bool polarity )
{
  boost::get_property( aig, boost::graph_name ).outputs += std::make_tuple( node, polarity, name );
}

aig_node aig_create_and( aig_graph& aig, aig_node left, aig_node right, bool polarity_left, bool polarity_right )
{
  /* sort left and right child */
  if ( left > right )
  {
    aig_node tmp = left;
    left = right;
    right = tmp;

    bool btmp = polarity_left;
    polarity_left = polarity_right;
    polarity_right = tmp;
  }

  auto& info = boost::get_property( aig, boost::graph_name );

  /* structural hashing */
  auto key = std::make_tuple( left, right, polarity_left, polarity_right );
  auto it = info.strash.find( key );
  if ( it != info.strash.end() )
  {
    return it->second;
  }

  /* create node and connect to children */
  aig_node node = add_vertex( aig );
  boost::get( boost::vertex_name, aig )[node] = 2u * num_vertices( aig );

  aig_edge le = add_edge( node, left, aig ).first;
  boost::get( boost::edge_polarity, aig )[le] = polarity_left;
  aig_edge re = add_edge( node, right, aig ).first;
  boost::get( boost::edge_polarity, aig )[re] = polarity_right;

  return info.strash[key] = node;
}

void write_dot( std::ostream& os, const aig_graph& aig )
{
  auto indexmap = get( boost::vertex_name, aig );

  boost::write_graphviz( os, aig, boost::make_label_writer( indexmap ) );
}

}

// Local Variables:
// c-basic-offset: 2
// End:
