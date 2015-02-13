/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
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

#include "simulation_graph.hpp"

#include <fstream>

#include <boost/assign/std/vector.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/range/algorithm.hpp>

#include <core/io/write_graph_file.hpp>
#include <core/utils/bitset_utils.hpp>
#include <core/utils/range_utils.hpp>
#include <classical/utils/simulate_aig.hpp>

using namespace boost::assign;

namespace cirkit
{

void create_simulation_graph( simulation_graph& g, const aig_graph& aig, const std::vector<boost::dynamic_bitset<>>& sim_vectors, properties::ptr settings )
{
  /* Settings */
  std::string dotname     = get( settings, "dotname",     std::string() );
  std::string graphname   = get( settings, "graphname",   std::string() );
  std::string labeledname = get( settings, "labeledname", std::string() );

  /* AIG */
  const auto& graph_info = boost::get_property( aig, boost::graph_name );

  /* add vertices */
  std::vector<simulation_node> nodes( graph_info.inputs.size() + sim_vectors.size() + graph_info.outputs.size() );
  boost::generate( nodes, [&g]() { return boost::add_vertex( g ); } );

  /* edges from inputs to simulation vectors */
  for ( unsigned i = 0u; i < sim_vectors.size(); ++i )
  {
    auto pos = sim_vectors[i].find_first();
    while ( pos != boost::dynamic_bitset<>::npos )
    {
      add_edge( nodes[pos], nodes[graph_info.inputs.size() + i], g );
      pos = sim_vectors[i].find_next( pos );
    }
  }

  /* simulate */
  auto sim_vectors_t = transpose( sim_vectors );
  word_node_assignment_simulator::aig_node_value_map m;
  for ( auto word : index( sim_vectors_t ) )
  {
    m[graph_info.inputs[word.first]] = word.second;
  }

  auto results = simulate_aig( aig, word_node_assignment_simulator( m ) );

  /* create edges */
  for ( unsigned i = 0; i < sim_vectors.size(); ++i )
  {
    for ( unsigned j = 0; j < graph_info.outputs.size(); ++j )
    {
      if ( results[graph_info.outputs[j].first][i] )
      {
        add_edge( nodes[graph_info.inputs.size() + i], nodes[graph_info.inputs.size() + sim_vectors.size() + j], g );
      }
    }
  }

  /* Dump files */
  if ( !dotname.empty() )
  {
    std::ofstream os( dotname.c_str(), std::ofstream::out );
    boost::write_graphviz( os, g );
    os.close();
  }

  if ( !graphname.empty() )
  {
    write_graph_file( g, graphname );
  }

  if ( !labeledname.empty() )
  {
    // TODO be more flexible
    write_labeled_graph_file( g, [&graph_info, &sim_vectors]( const simulation_node& v ) {
        if ( v < graph_info.inputs.size() ) return 1u;
        if ( v < graph_info.inputs.size() + sim_vectors.size() ) return 2u + (unsigned)std::min( (unsigned)sim_vectors[v - graph_info.inputs.size()].count(), 3u );
        return 0u;
      }, [&g, &graph_info, &sim_vectors]( const simulation_edge& e ) {
        if ( boost::source( e, g ) < graph_info.inputs.size() ) return 0u;
        else return 1u + (unsigned)std::min( (unsigned)sim_vectors[boost::source( e, g ) - graph_info.inputs.size()].count(), 3u );
      }, labeledname );
  }
}

void create_simulation_vectors( std::vector<boost::dynamic_bitset<>>& sim_vectors, unsigned width, unsigned selector,
                                std::vector<unsigned>* partition )
{
  if ( selector & static_cast<unsigned>( simulation_pattern::all_hot ) )
  {
    sim_vectors += ~boost::dynamic_bitset<>( width );
    if ( partition ) *partition += 1;
  }
  if ( selector & static_cast<unsigned>( simulation_pattern::one_hot ) )
  {
    for ( unsigned i = 0u; i < width; ++i )
    {
      boost::dynamic_bitset<> b( width );
      b.set( i );
      sim_vectors += b;
    }
    if ( partition ) *partition += width;
  }
  if ( selector & static_cast<unsigned>( simulation_pattern::two_hot ) )
  {
    for ( unsigned i = 0u; i < width - 1u; ++i )
    {
      for ( unsigned j = i + 1u; j < width; ++j )
      {
        boost::dynamic_bitset<> b( width );
        b.set( i );
        b.set( j );
        sim_vectors += b;
      }
    }
    if ( partition ) *partition += ( width * (width -  1) ) / 2;
  }

  if ( selector & static_cast<unsigned>( simulation_pattern::all_cold ) )
  {
    sim_vectors += boost::dynamic_bitset<>( width );
    if ( partition ) *partition += 1;
  }
  if ( selector & static_cast<unsigned>( simulation_pattern::one_cold ) )
  {
    for ( unsigned i = 0u; i < width; ++i )
    {
      auto b = ~boost::dynamic_bitset<>( width );
      b.reset( i );
      sim_vectors += b;
    }
    if ( partition ) *partition += width;
  }
  if ( selector & static_cast<unsigned>( simulation_pattern::two_cold ) )
  {
    for ( unsigned i = 0u; i < width - 1u; ++i )
    {
      for ( unsigned j = i + 1u; j < width; ++j )
      {
        auto b = ~boost::dynamic_bitset<>( width );
        b.reset( i );
        b.reset( j );
        sim_vectors += b;
      }
    }
    if ( partition ) *partition += ( width * (width -  1) ) / 2;
  }
}

igraph_t simulation_graph_to_igraph( const simulation_graph& g )
{
  igraph_t ig;
  igraph_vector_t edges;

  igraph_vector_init( &edges, boost::num_edges( g ) << 1u );
  unsigned index = 0u;
  for ( const auto& e : boost::make_iterator_range( boost::edges( g ) ) )
  {
    VECTOR(edges)[index++] = boost::source( e, g );
    VECTOR(edges)[index++] = boost::target( e, g );
  }
  igraph_create( &ig, &edges, boost::num_vertices( g ), 1 );
  igraph_simplify( &ig, 1, 1, 0 );
  return ig;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
