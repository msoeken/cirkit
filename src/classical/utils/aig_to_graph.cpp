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

#include "aig_to_graph.hpp"

#include <climits>
#include <map>

#include <boost/graph/graph_traits.hpp>

namespace revkit
{

  void aig_to_graph( const aiger * aig, aig_graph& graph )
  {
    typedef boost::graph_traits<aig_graph>::vertex_descriptor vertex_t;
    typedef boost::graph_traits<aig_graph>::edge_descriptor edge_t;

    std::map<unsigned, vertex_t> id_to_vertex;
    boost::property_map<aig_graph, boost::vertex_name_t>::type indexmap = get( boost::vertex_name, graph );
    boost::property_map<aig_graph, boost::edge_polarity_t>::type polaritymap = get( boost::edge_polarity, graph );
    boost::property_map<aig_graph, boost::edge_weight_t>::type weightmap = get( boost::edge_weight, graph );

    vertex_t source = add_vertex( graph );
    vertex_t target = add_vertex( graph );

    for ( unsigned i = 0u; i < aig->num_inputs; ++i )
    {
      vertex_t vertex = add_vertex( graph );
      indexmap[vertex] = aig->inputs[i].lit;
      id_to_vertex[aig->inputs[i].lit] = vertex;

      edge_t edge = add_edge( source, vertex, graph ).first;
      polaritymap[edge] = true;
      weightmap[edge] = UINT_MAX;
    }

    for ( unsigned i = 0u; i < aig->num_ands; ++i )
    {
      aiger_and * node = aig->ands + i;

      vertex_t vertex = add_vertex( graph );
      indexmap[vertex] = aiger_strip( node->lhs );
      id_to_vertex.insert( std::make_pair( aiger_strip( node->lhs ), vertex ) );

      edge_t edge0 = add_edge( id_to_vertex[aiger_strip( node->rhs0 )], vertex, graph ).first;
      polaritymap[edge0] = !( node->rhs0 & 1 );
      weightmap[edge0] = 1u;
      edge_t edge1 = add_edge( id_to_vertex[aiger_strip( node->rhs1 )], vertex, graph ).first;
      polaritymap[edge1] = !( node->rhs0 & 1 );
      weightmap[edge0] = 1u;
    }

    for ( unsigned i = 0u; i < aig->num_outputs; ++i )
    {
      edge_t edge = add_edge( id_to_vertex[aiger_strip( aig->outputs[i].lit )], target, graph ).first;
      polaritymap[edge] = !( aig->outputs[i].lit & 1 );
      weightmap[edge] = UINT_MAX;
    }
  }

}

// Local Variables:
// c-basic-offset: 2
// End:
