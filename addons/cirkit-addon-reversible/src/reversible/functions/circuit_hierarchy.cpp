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
