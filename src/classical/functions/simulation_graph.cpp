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
#include <boost/format.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
#include <boost/range/combine.hpp>
#include <boost/range/iterator_range.hpp>

#include <core/io/write_graph_file.hpp>
#include <core/utils/bitset_utils.hpp>
#include <core/utils/range_utils.hpp>
#include <classical/functions/aig_support.hpp>
#include <classical/utils/aig_utils.hpp>
#include <classical/utils/simulate_aig.hpp>

using namespace boost::assign;

namespace cirkit
{

simulation_graph create_simulation_graph( const aig_graph& aig, const std::vector<boost::dynamic_bitset<>>& sim_vectors,
                                          const properties::ptr& settings, const properties::ptr& statistics )
{
  simulation_graph g;
  auto& edge_lookup             = boost::get_property( g, boost::graph_edge_lookup );
  auto& meta                    = boost::get_property( g, boost::graph_meta );
  const auto& vertex_in_degree  = boost::get( boost::vertex_in_degree, g );
  const auto& vertex_out_degree = boost::get( boost::vertex_out_degree, g );

  /* Settings */
  const auto dotname               = get( settings, "dotname",               std::string() );
  const auto graphname             = get( settings, "graphname",             std::string() );
  const auto labeledname           = get( settings, "labeledname",           std::string() );
  const auto support               = get( settings, "support",               false );
  const auto support_edges         = get( settings, "support_edges",         false );
  const auto vertexnames           = get( settings, "vertexnames",           false );
  const auto simulation_signatures = get( settings, "simulation_signatures", false );

  if ( support_edges && !support )
  {
    std::cout << "[w] support_edges can only be actived when support is actived; setting will be ignored" << std::endl;
  }

  /* Timing */
  properties_timer t( statistics );

  /* AIG */
  const auto& info = aig_info( aig );
  const auto n = info.inputs.size();
  const auto m = info.outputs.size();

  /* meta */
  meta.num_inputs = n;
  meta.num_vectors = sim_vectors.size();
  meta.num_outputs = m;

  /* add vertices */
  const auto vertices_count = n + sim_vectors.size() + m;
  add_vertices( g, vertices_count );
  edge_lookup.reserve( vertices_count << 5u );

  /* edge inserting */
  const auto add_edge_func = [&]( unsigned from, unsigned to ) {
    auto e = add_edge( from, to, g ).first;
    edge_lookup.insert( std::make_pair( std::make_pair( from, to ), e ) );
    edge_lookup.insert( std::make_pair( std::make_pair( to, from ), e ) );
    vertex_in_degree[to]++;
    vertex_out_degree[from]++;
  };

  /* edges from inputs to simulation vectors */
  for ( auto it : index( sim_vectors ) )
  {
    foreach_bit( it.second, [&]( unsigned pos ) {
        add_edge_func( pos, n + it.first );
      } );
  }

  /* simulate */
  word_assignment_simulator::aig_name_value_map map( info.inputs.size() );
  const auto sim_vectors_t = transpose( sim_vectors );
  for ( const auto& p : boost::combine( info.inputs, sim_vectors_t ) )
  {
    map.insert( {info.node_names.at( boost::get<0>( p ) ), boost::get<1>( p )} );
  }

  const auto results = simulate_aig( aig, word_assignment_simulator( map ) );

  /* create edges */
  for ( auto j = 0u; j < m; ++j )
  {
    const auto& ovalue = results.at( info.outputs[j].first );
    for ( auto i = 0u; i < sim_vectors.size(); ++i )
    {
      if ( ovalue[i] )
      {
        add_edge_func( n + i, n + sim_vectors.size() + j );
      }
    }
  }

  /* annotate support size */
  if ( support )
  {
    const auto& vertex_support = boost::get( boost::vertex_support, g );

    const auto s = aig_structural_support( aig );
    for ( const auto& o : index( info.outputs ) )
    {
      const auto& mask = s.at( o.second.first );
      vertex_support[n + sim_vectors.size() + o.first] = mask.count();

      if ( support_edges )
      {
        auto it_bit = mask.find_first();
        while ( it_bit != boost::dynamic_bitset<>::npos )
        {
          add_edge_func( n + sim_vectors.size() + o.first, it_bit );
          it_bit = mask.find_next( it_bit );
        }
      }
    }
  }

  /* add vertex names */
  if ( vertexnames )
  {
    const auto& vertex_names = boost::get( boost::vertex_name, g );

    for ( const auto& i : index( info.inputs ) )
    {
      vertex_names[i.first] = info.node_names.at( i.second );
    }

    std::string bitstring;
    for ( const auto& v : index( sim_vectors ) )
    {
      boost::to_string( v.second, bitstring );
      vertex_names[n + v.first] = bitstring;
    }

    for ( const auto& o : index( info.outputs ) )
    {
      vertex_names[n + sim_vectors.size() + o.first] = o.second.second;
    }
  }

  /* simulation signatures */
  if ( simulation_signatures )
  {
    const auto& vertex_simulation_signatures = boost::get( boost::vertex_simulation_signature, g );

    word_assignment_simulator::aig_name_value_map map( info.inputs.size() );
    std::vector<unsigned> partition, offset( 6u );
    const auto sim_vectors   = create_simulation_vectors( n, 63u /* all */, &partition );
    const auto sim_vectors_t = transpose( sim_vectors );

    assert( partition.size() == 6u );
    offset[0] = 0;
    for ( auto i = 1u; i < partition.size(); ++i )
    {
      offset[i] = offset[i - 1] + partition[i - 1];
    }

    for ( const auto& p : boost::combine( info.inputs, sim_vectors_t ) )
    {
      map.insert( {info.node_names.at( boost::get<0>( p ) ), boost::get<1>( p )} );
    }

    const auto results = simulate_aig( aig, word_assignment_simulator( map ) );

    for ( auto j = 0u; j < m; ++j )
    {
      const auto& ovalue = results.at( info.outputs.at( j ).first );
      std::array<unsigned, 6u> signature;
      for ( auto i = 0u; i < partition.size(); ++i )
      {
        signature[i] = 0u;
        auto pos = ovalue.find_next( offset[i] );
        while ( pos < offset[i] + partition[i] && pos != boost::dynamic_bitset<>::npos )
        {
          ++signature[i];
          pos = ovalue.find_next( pos );
        }
      }

      std::cout << "[i] signature for " << info.outputs[j].second << ": " << any_join( signature, " " ) << std::endl;
      vertex_simulation_signatures[n + sim_vectors.size() + j] = signature;
    }
  }

  /* Dump files */
  if ( !dotname.empty() )
  {
    std::ofstream os( dotname.c_str(), std::ofstream::out );
    if ( vertexnames )
    {
      boost::write_graphviz( os, g, boost::make_label_writer( boost::get( boost::vertex_name, g ) ) );
    }
    else
    {
      boost::write_graphviz( os, g );
    }
    os.close();
  }

  if ( !graphname.empty() )
  {
    assert( false ); /* write_graph_file is written for directed graphs */
    write_graph_file( g, graphname );
  }

  if ( !labeledname.empty() )
  {
    assert( false ); /* write_labeled_graph_file is written for directed graphs */

    // TODO be more flexible
    write_labeled_graph_file( g, [&]( const simulation_node& v ) {
        if ( v < n ) return 1u;
        if ( v < n + sim_vectors.size() ) return 2u + (unsigned)std::min( (unsigned)sim_vectors[v - n].count(), 3u );
        return 0u;
      }, [&]( const simulation_edge& e ) {
        if ( boost::source( e, g ) < n ) return 0u;
        else return 1u + (unsigned)std::min( (unsigned)sim_vectors[boost::source( e, g ) - n].count(), 3u );
      }, labeledname );
  }

  return g;
}

std::vector<boost::dynamic_bitset<>> create_simulation_vectors( unsigned width, unsigned selector,
                                                                std::vector<unsigned>* partition )
{
  std::vector<boost::dynamic_bitset<>> sim_vectors;

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

  return sim_vectors;
}

simulation_graph create_simulation_graph( const aig_graph& aig, unsigned selector,
                                          const properties::ptr& settings,
                                          const properties::ptr& statistics )
{
  auto additional_vectors = get( settings, "additional_vectors", std::vector<boost::dynamic_bitset<>>() );
  auto support_edges      = get( settings, "support_edges",      false );

  std::vector<unsigned> partition;

  const auto& info    = aig_info( aig );
  const auto  n       = info.inputs.size();
  const auto  m       = info.outputs.size();
  auto        vectors = create_simulation_vectors( info.inputs.size(), selector, &partition );

  boost::push_back( vectors, additional_vectors );

  auto        graph   = create_simulation_graph( aig, vectors, settings, statistics );

  properties_timer t( statistics, "labeling_runtime" );

  const auto& vertex_label = boost::get( boost::vertex_label, graph );
  const auto& edge_label   = boost::get( boost::edge_label, graph );

  for ( auto i = 0; i < n; ++i )
  {
    vertex_label[i] = 0u;
  }
  for ( auto i = 0; i < m; ++i )
  {
    vertex_label[n + vectors.size() + i] = 1u;

    /* edges (Y -> X) */
    if ( support_edges )
    {
      for ( const auto& edge : boost::make_iterator_range( boost::out_edges( n + vectors.size() + i, graph ) ) )
      {
        edge_label[edge] = 2u * partition.size();
      }
    }
  }

  /* edges ( X -> sim -> Y) */
  auto offset = n;
  for ( const auto& p : index( partition ) )
  {
    for ( auto i = 0; i < p.second; ++i )
    {
      vertex_label[offset + i] = 2u + p.first;

      for ( const auto& edge : boost::make_iterator_range( boost::out_edges( offset + i, graph ) ) )
      {
        const auto is_syedge = ( boost::target( edge, graph ) > offset + i ) ? 1u : 0u;
        edge_label[edge] = is_syedge * partition.size() + p.first;
      }
    }
    offset += p.second;
  }

  return graph;
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
