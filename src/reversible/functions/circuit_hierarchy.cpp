/* RevKit (www.rekit.org)
 * Copyright (C) 2009-2014  University of Bremen
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

#include "circuit_hierarchy.hpp"

#include "../circuit.hpp"

namespace cirkit
{

  typedef boost::graph_traits<hierarchy_graph>::vertex_descriptor hierarchy_vertex;

  hierarchy_vertex _circuit_hierarchy( const circuit& circ, const std::string& name, hierarchy_graph& graph )
  {
    hierarchy_vertex v = boost::add_vertex( graph );
    boost::get<0>( boost::get( boost::vertex_name, graph )[v] ) = name;
    boost::get<1>( boost::get( boost::vertex_name, graph )[v] ) = &circ;

    for ( const auto& module : circ.modules() )
    {
      boost::add_edge( v, _circuit_hierarchy( *module.second, module.first, graph ), graph );
    }

    return v;
  }

  void circuit_hierarchy( const circuit& circ, hierarchy_tree& tree )
  {
    hierarchy_tree::node_type root = _circuit_hierarchy( circ, "top", tree.graph() );
    tree.set_root( root );
  }

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
