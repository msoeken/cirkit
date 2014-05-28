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
#include <fstream>
#include <map>

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graphviz.hpp>

namespace revkit
{

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

typedef boost::graph_traits<aig_graph>::vertex_descriptor vertex_t;
typedef boost::graph_traits<aig_graph>::edge_descriptor edge_t;

void add_edge_with_rev( vertex_t v, vertex_t w, aig_graph& graph, double weight, bool polarity )
{
  auto polaritymap = get( boost::edge_polarity, graph );
  auto capacitymap = get( boost::edge_capacity, graph );
  auto reversemap  = get( boost::edge_reverse,  graph );

  edge_t edge  = add_edge( v, w, graph ).first;
  edge_t redge = add_edge( w, v, graph ).first;

  polaritymap[edge] = polaritymap[redge] = polarity;
  capacitymap[edge] = weight;
  capacitymap[redge] = 0.0;
  reversemap[edge] = redge;
  reversemap[redge] = edge;
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

aig_to_graph_settings::aig_to_graph_settings() {}

void aig_to_graph( const aiger * aig, aig_graph& graph, const aig_to_graph_settings& settings )
{
  std::map<unsigned, vertex_t> id_to_vertex;
  vertex_t source, target;

  auto indexmap    = get( boost::vertex_name,   graph );
  auto capacitymap = get( boost::edge_capacity, graph );

  indexmap[source = add_vertex( graph )] = 0u;
  indexmap[target = add_vertex( graph )] = 1u;

  /* Add inputs to graph */
  for ( unsigned i = 0u; i < aig->num_inputs; ++i )
  {
    vertex_t vertex = add_vertex( graph );
    indexmap[vertex] = aig->inputs[i].lit;
    id_to_vertex[aig->inputs[i].lit] = vertex;

    add_edge_with_rev( source, vertex, graph, std::numeric_limits<double>::infinity(), true );
  }

  /* Add vertex for each AND node */
  for ( unsigned i = 0u; i < aig->num_ands; ++i )
  {
    aiger_and * node = aig->ands + i;

    vertex_t vertex = add_vertex( graph );
    indexmap[vertex] = aiger_strip( node->lhs );
    id_to_vertex.insert( std::make_pair( aiger_strip( node->lhs ), vertex ) );
  }

  /* Connect AND nodes */
  for ( unsigned i = 0u; i < aig->num_ands; ++i )
  {
    aiger_and * node = aig->ands + i;

    add_edge_with_rev( id_to_vertex[aiger_strip( node->rhs0 )], id_to_vertex[aiger_strip( node->lhs )], graph, 1.0, !( node->rhs0 & 1 ) );
    add_edge_with_rev( id_to_vertex[aiger_strip( node->rhs1 )], id_to_vertex[aiger_strip( node->lhs )], graph, 1.0, !( node->rhs1 & 1 ) );
  }

  /* Connect outputs to target */
  for ( unsigned i = 0u; i < aig->num_outputs; ++i )
  {
    add_edge_with_rev( id_to_vertex[aiger_strip( aig->outputs[i].lit )], target, graph, std::numeric_limits<double>::infinity(), !( aig->outputs[i].lit & 1 ) );
  }

  if ( !settings.dotname.empty() )
  {
    std::filebuf fb;
    fb.open( settings.dotname.c_str(), std::ios::out );
    std::ostream os( &fb );
    boost::write_graphviz( os, graph, boost::make_label_writer( indexmap ), boost::make_label_writer( capacitymap ) );
    fb.close();
  }
}

}

// Local Variables:
// c-basic-offset: 2
// End:
