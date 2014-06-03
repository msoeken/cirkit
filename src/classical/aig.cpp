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

aig_function aig_get_constant( aig_graph& aig, bool value )
{
  return std::make_pair( 0u, value );
}

aig_function aig_create_pi( aig_graph& aig, const std::string& name )
{
  aig_node node = add_vertex( aig );
  boost::get( boost::vertex_name, aig )[node] = 2u * num_vertices( aig );

  boost::get_property( aig, boost::graph_name ).inputs += node;
  boost::get_property( aig, boost::graph_name ).node_names[node] = name;

  return std::make_pair( node, true );
}

void aig_create_po( aig_graph& aig, const aig_function& f, const std::string& name )
{
  boost::get_property( aig, boost::graph_name ).outputs += std::make_pair( f, name );
}

aig_function aig_create_and( aig_graph& aig, aig_function left, aig_function right )
{
  /* sort left and right child */
  if ( left.first > right.first )
  {
    aig_function tmp = left;
    left = right;
    right = tmp;
  }

  auto& info = boost::get_property( aig, boost::graph_name );

  /* structural hashing */
  auto key = std::make_pair( left, right );
  auto it = info.strash.find( key );
  if ( it != info.strash.end() )
  {
    return it->second;
  }

  /* create node and connect to children */
  aig_node node = add_vertex( aig );
  boost::get( boost::vertex_name, aig )[node] = 2u * num_vertices( aig );

  aig_edge le = add_edge( node, left.first, aig ).first;
  boost::get( boost::edge_polarity, aig )[le] = left.second;
  aig_edge re = add_edge( node, right.first, aig ).first;
  boost::get( boost::edge_polarity, aig )[re] = right.second;

  return info.strash[key] = std::make_pair( node, true );
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
