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

#include "simulation_graph.hpp"

#include <fstream>

#include <boost/assign/std/vector.hpp>
#include <boost/format.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/optional.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext/iota.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
#include <boost/range/combine.hpp>
#include <boost/range/counting_range.hpp>
#include <boost/range/iterator_range.hpp>

#include <core/io/write_graph_file.hpp>
#include <core/utils/combinations.hpp>
#include <core/utils/range_utils.hpp>
#include <core/utils/string_utils.hpp>
#include <classical/functions/simulate_aig.hpp>
#include <classical/functions/aig_support.hpp>
#include <classical/utils/aig_utils.hpp>

using namespace boost::assign;

namespace cirkit
{

simulation_graph create_simulation_graph( const aig_graph& aig, const std::vector<boost::dynamic_bitset<>>& sim_vectors,
                                          const properties::ptr& settings, const properties::ptr& statistics )
{
  simulation_graph g;
  auto& meta                    = boost::get_property( g, boost::graph_meta );
  const auto& vertex_in_degree  = boost::get( boost::vertex_in_degree, g );
  const auto& vertex_out_degree = boost::get( boost::vertex_out_degree, g );

  /* Settings */
  const auto dotname               = get( settings, "dotname",               std::string() );
  const auto graphname             = get( settings, "graphname",             std::string() );
  const auto labeledname           = get( settings, "labeledname",           std::string() );
  const auto support               = get( settings, "support",               false );
  const auto support_edges         = get( settings, "support_edges",         false );
  const auto simulation_signatures = get( settings, "simulation_signatures", boost::optional<unsigned>() );
  const auto annotate_simvectors   = get( settings, "annotate_simvectors",   false );

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
  const auto vertices_count = n + m + sim_vectors.size();
  add_vertices( g, vertices_count );

  /* edge inserting */
  const auto add_edge_func = [&]( unsigned from, unsigned to ) {
    auto e = add_edge( from, to, g ).first;
    vertex_in_degree[to]++;
    vertex_out_degree[from]++;
    return e;
  };

  /* edges from inputs to simulation vectors */
  for ( auto it : index( sim_vectors ) )
  {
    foreach_bit( it.value, [&]( unsigned pos ) {
        add_edge_func( pos, n + it.index );
      } );
  }

  /* simulate */
  word_assignment_simulator::aig_name_value_map map( info.inputs.size() );
  const auto sim_vectors_t = transpose( sim_vectors );
  for ( const auto& p : boost::combine( info.inputs, sim_vectors_t ) )
  {
    map.insert( {info.node_names.at( boost::get<0>( p ) ), boost::get<1>( p )} );
  }

  auto results = simulate_aig( aig, word_assignment_simulator( map ) );

  /* prepare annotation of simvectors */
  std::vector<boost::dynamic_bitset<>> results_t;

  if ( annotate_simvectors )
  {
    results_t.resize( sim_vectors.size(), boost::dynamic_bitset<>( m ) );
  }

  /* create edges */
  for ( auto j = 0u; j < m; ++j )
  {
    const auto& ovalue = results[info.outputs[j].first];
    for ( auto i = 0u; i < sim_vectors.size(); ++i )
    {
      if ( ovalue[i] )
      {
        add_edge_func( n + i, n + sim_vectors.size() + j );
      }

      if ( annotate_simvectors )
      {
        results_t[i][j] = ovalue[i];
      }
    }
  }

  /* annotate support size */
  if ( support )
  {
    const auto& vertex_support = boost::get( boost::vertex_support, g );
    const auto& edge_kind      = boost::get( boost::edge_kind, g );

    support_map_t s;
    boost::dynamic_bitset<> unate;

    if ( info.unateness.empty() )
    {
      s = aig_structural_support( aig );
    }
    else
    {
      s = aig_functional_support( aig );
      unate = info.unateness;
    }

    for ( const auto& o : index( info.outputs ) )
    {
      const auto& mask = s.at( o.value.first );
      vertex_support[n + sim_vectors.size() + o.index] = mask;

      if ( support_edges )
      {
        auto it_bit = mask.find_first();
        while ( it_bit != boost::dynamic_bitset<>::npos )
        {
          const auto e = add_edge_func( n + sim_vectors.size() + o.index, it_bit );

          if ( !unate.empty() )
          {
            edge_kind[e] = get_unateness_kind( unate, o.index, it_bit, n );
          }

          it_bit = mask.find_next( it_bit );
        }
      }
    }
  }

  /* port to node mapping */
  for ( const auto& i : index( info.inputs ) )
  {
    meta.port_to_node[i.value] = i.index;
  }

  for ( const auto& o : index( info.outputs ) )
  {
    meta.port_to_node[o.value.first.node] = n + sim_vectors.size() + o.index;
  }

  /* annotate simulation vectors */
  if ( annotate_simvectors )
  {
    const auto& vertex_sim_vector = boost::get( boost::vertex_simulation_vector, g );
    const auto& vertex_sim_result = boost::get( boost::vertex_simulation_result, g );

    for ( const auto& v : index( sim_vectors ) )
    {
      vertex_sim_vector[n + v.index] = v.value;
      vertex_sim_result[n + v.index] = results_t[v.index];
    }
  }

  /* simulation signatures */
  if ( (bool)simulation_signatures )
  {
    const auto& vertex_simulation_signatures = boost::get( boost::vertex_simulation_signature, g );

    const auto signatures = compute_simulation_signatures( aig, *simulation_signatures );
    for ( const auto& s : index( signatures ) )
    {
      vertex_simulation_signatures[n + sim_vectors.size() + s.index] = s.value;
    }
  }

  /* Dump files */
  if ( !dotname.empty() )
  {
    std::ofstream os( dotname.c_str(), std::ofstream::out );

    // TODO consider vertex names in a different way
    boost::write_graphviz( os, g );
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

std::vector<boost::dynamic_bitset<>> create_simulation_vectors( unsigned width, const std::vector<unsigned>& types,
                                                                std::vector<unsigned>* partition )
{
  std::vector<boost::dynamic_bitset<>> sim_vectors;

  std::vector<unsigned> numbers( width );
  boost::iota( numbers, 0u );

  for ( const auto& j : types )
  {
    const auto k   = j / 2;
    const auto hot = ( j % 2 == 1 );

    boost::unofficial::for_each_combination( numbers.begin(), numbers.begin() + k, numbers.end(),
                                             [&]( std::vector<unsigned>::const_iterator first,
                                                  std::vector<unsigned>::const_iterator last )
                                             {
                                               boost::dynamic_bitset<> b( width );

                                               while ( first != last )
                                               {
                                                 b.set( *first );
                                                 ++first;
                                               }

                                               if ( !hot ) { b.flip(); }
                                               sim_vectors += b;

                                               return false;
                                             } );

    if ( partition )
    {
      *partition += boost::unofficial::count_each_combination( k, width - k );
    }
  }

  return sim_vectors;
}

simulation_graph create_simulation_graph( const aig_graph& aig, const std::vector<unsigned>& types,
                                          const properties::ptr& settings,
                                          const properties::ptr& statistics )
{
  auto additional_vectors = get( settings, "additional_vectors", std::vector<boost::dynamic_bitset<>>() );
  auto support_edges      = get( settings, "support_edges",      false );

  std::vector<unsigned> partition;

  const auto& info    = aig_info( aig );
  const auto  n       = info.inputs.size();
  const auto  m       = info.outputs.size();

  std::vector<boost::dynamic_bitset<>> vectors;

  {
    properties_timer t( statistics, "simulation_runtime" );
    vectors = create_simulation_vectors( info.inputs.size(), types, &partition );
  }

  boost::push_back( vectors, additional_vectors );

  set( statistics, "vectors", vectors );

  auto        graph   = create_simulation_graph( aig, vectors, settings, statistics );

  properties_timer t( statistics, "labeling_runtime" );

  const auto& vertex_label = boost::get( boost::vertex_label, graph );
  const auto& edge_label   = boost::get( boost::edge_label, graph );

  for ( auto i = 0u; i < n; ++i )
  {
    vertex_label[i] = 0u;
  }
  for ( auto i = 0u; i < m; ++i )
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
    for ( auto i = 0u; i < p.value; ++i )
    {
      vertex_label[offset + i] = 2u + p.index;

      for ( const auto& edge : boost::make_iterator_range( boost::out_edges( offset + i, graph ) ) )
      {
        const auto is_syedge = ( boost::target( edge, graph ) > offset + i ) ? 1u : 0u;
        edge_label[edge] = is_syedge * partition.size() + p.index;
      }
    }
    offset += p.value;
  }

  return graph;
}

std::vector<simulation_signature_t::value_type> compute_simulation_signatures( const aig_graph& aig, unsigned maxk )
{
  std::vector<simulation_signature_t::value_type> vec;

  const auto& info      = aig_info( aig );
  const auto  n         = info.inputs.size();
  const auto  num_types = ( maxk + 1u ) << 1u;

  std::vector<unsigned> types( num_types );
  boost::iota( types, 0u );

  word_assignment_simulator::aig_name_value_map map( n );
  std::vector<unsigned> partition, offset( num_types );
  const auto all_sim_vectors   = create_simulation_vectors( n, types, &partition );
  const auto all_sim_vectors_t = transpose( all_sim_vectors );

  assert( partition.size() == num_types );
  offset[0] = 0;
  for ( auto i = 1u; i < partition.size(); ++i )
  {
    offset[i] = offset[i - 1] + partition[i - 1];
  }

  for ( const auto& p : boost::combine( info.inputs, all_sim_vectors_t ) )
  {
    map.insert( {info.node_names.at( boost::get<0>( p ) ), boost::get<1>( p )} );
  }

  const auto results = simulate_aig( aig, word_assignment_simulator( map ) );

  for ( const auto& output : info.outputs )
  {
    const auto& ovalue = results.at( output.first );
    std::vector<unsigned> signature( num_types );
    for ( auto i = 0u; i < partition.size(); ++i )
    {
      signature[i] = 0u;
      auto pos = ( i == 0u ) ? ovalue.find_first() : ovalue.find_next( offset[i] - 1u );
      while ( pos < offset[i] + partition[i] && pos != boost::dynamic_bitset<>::npos )
      {
        ++signature[i];
        pos = ovalue.find_next( pos );
      }
    }

    vec += signature;
  }

  return vec;
}

/******************************************************************************
 * simulation_graph_wrapper                                                   *
 ******************************************************************************/

inline simulation_graph create_simulation_graph_wrapper( const aig_graph& aig, const std::vector<unsigned>& types,
                                                         bool support_edges, const boost::optional<unsigned>& simulation_signatures )
{
  const auto settings = std::make_shared<properties>();
  settings->set( "support", support_edges );
  settings->set( "support_edges", support_edges );
  settings->set( "simulation_signatures", simulation_signatures );
  settings->set( "annotate_simvectors", true );

  return create_simulation_graph( aig, types, settings );
}

simulation_graph_wrapper::simulation_graph_wrapper( const aig_graph& g,
                                                    const std::vector<unsigned>& types,
                                                    bool support_edges,
                                                    const boost::optional<unsigned>& simulation_signatures )
  : aig( g ),
    info( aig_info( g ) ),
    graph( create_simulation_graph_wrapper( g, types, support_edges, simulation_signatures ) ),
#ifdef FAST_EDGE_ACCESS
    vedge_label( boost::num_vertices( graph ), std::vector<int>( boost::num_vertices( graph ), 0 ) ),
    vedge_direction( boost::num_vertices( graph ), std::vector<int>( boost::num_vertices( graph ), 0 ) )
#else
    vedge_label( boost::num_vertices( graph ) ),
    vedge_direction( boost::num_vertices( graph ) )
#endif
{
  std::cout << "[i] start sg wrapper" << std::endl;

  vertex_label                = boost::get( boost::vertex_label, graph );
  vertex_in_degree            = boost::get( boost::vertex_in_degree, graph );
  vertex_out_degree           = boost::get( boost::vertex_out_degree, graph );
  vertex_support              = boost::get( boost::vertex_support, graph );
  vertex_simulation_signature = boost::get( boost::vertex_simulation_signature, graph );
  vertex_sim_vectors          = boost::get( boost::vertex_simulation_vector, graph );
  medge_label                 = boost::get( boost::edge_label, graph );

  /* fill labels */
  for ( const auto& e : edges() )
  {
    const auto& src = boost::source( e, graph );
    const auto& tgt = boost::target( e, graph );

#ifdef FAST_EDGE_ACCESS
    vedge_label[src][tgt] = vedge_label[tgt][src] = medge_label[e];
    vedge_direction[src][tgt] = 1;
    vedge_direction[tgt][src] = 2;
#else
    vedge_label[src].insert( {tgt, medge_label[e]} );
    vedge_label[tgt].insert( {src, medge_label[e]} );
    vedge_direction[src].insert( {tgt, 1} );
    vedge_direction[tgt].insert( {src, 1} );
#endif
  }

  std::cout << "[i] end sg wrapper" << std::endl;
}

void simulation_graph_wrapper::fill_neighbor_degree_sequence_out( unsigned u, std::vector<unsigned>& degrees ) const
{
  assert( degrees.empty() );
  assert( out_degree( u ) + in_degree( u ) == degree( u ) );
  for ( const auto& u2 : adjacent( u ) )
  {
    if ( ( is_input( u ) && is_vector( u2 ) )
         || ( is_vector( u ) && is_output( u2 ) )
         || ( is_output( u ) && is_input( u2 ) ) )
    {
      degrees.push_back( out_degree( u2 ) );
    }
  }
  assert( degrees.size() == out_degree( u ) );
  boost::sort( degrees );
}

void simulation_graph_wrapper::fill_neighbor_degree_sequence_all( unsigned u, std::vector<unsigned>& degrees ) const
{
  degrees.resize( degree( u ) );
  boost::transform( adjacent( u ), degrees.begin(), [&]( unsigned u2 ) { return degree( u2 ); } );
  boost::sort( degrees );
}

void simulation_graph_wrapper::add_edge_kinds()
{
  vedge_kind.resize( num_outputs() );

  const auto& edge_kind = boost::get( boost::edge_kind, graph );

  for ( const auto& out : output_indexes() )
  {
    for ( const auto& e : out_edges( out ) )
    {
      vedge_kind[output_index( out )][input_index( boost::target( e, graph ) )] = edge_kind[e];
    }
  }
}

struct simulation_graph_wrapper_dot_writer
{
public:
  explicit simulation_graph_wrapper_dot_writer( const simulation_graph_wrapper& simgraph ) : simgraph( simgraph ) {}

  void operator()( std::ostream& out, const simulation_node& v ) const
  {
    string_properties_map_t properties = {{"label", boost::str( boost::format( "\"%s\"" ) % simgraph.name( v ) )}};

    if ( simgraph.is_input( v ) )
    {
      properties.insert( {"shape", "cds"} );
    }
    else if ( simgraph.is_output( v ) )
    {
      properties.insert( {"shape", "cds"} );
      properties.insert( {"orientation", "180"} );
    }
    out << boost::format( "[%s]" ) % make_properties_string( properties );
  }

  void operator()( std::ostream& out, const simulation_edge& e ) const
  {
    if ( ( simgraph.is_input( simgraph.source( e ) ) && simgraph.is_output( simgraph.target( e ) ) ) ||
         ( simgraph.is_output( simgraph.source( e ) ) && simgraph.is_input( simgraph.target( e ) ) ) )
    {
      out << boost::format( "[color=gray]" );
    }
  }

  void operator()( std::ostream& out ) const
  {
    const auto n = simgraph.num_inputs();
    const auto v = simgraph.num_vectors();
    const auto m = simgraph.num_outputs();

    out << "rankdir=LR" << std::endl
        << "{rank=same " << any_join( boost::counting_range( 0u, n ), " " ) << " }" << std::endl
        << "{rank=same " << any_join( boost::counting_range( n, n + v ), " " ) << " }" << std::endl
        << "{rank=same " << any_join( boost::counting_range( n + v, n + v + m ), " " ) << " }" << std::endl;
  }

private:
  const simulation_graph_wrapper& simgraph;
};

void simulation_graph_wrapper::write_dot( const std::string& filename ) const
{
  std::ofstream os( filename.c_str(), std::ofstream::out );

  simulation_graph_wrapper_dot_writer writer( *this );
  boost::write_graphviz( os, graph, writer, writer, writer );
}

bool compatible_simulation_signatures( const simulation_graph_wrapper& pg, const simulation_graph_wrapper& tg,
                                       const simulation_node& u, const simulation_node& v,
                                       unsigned maxk )
{
  const auto& sigp = pg.simulation_signature( u );
  const auto& sigt = tg.simulation_signature( v );

  const auto nmink = tg.num_inputs() - pg.num_inputs();

  /* compute binomial coeffecients */
  std::vector<unsigned> coeffs( maxk + 1u );
  coeffs[0u] = 1u;
  for ( auto k = 0u; k < coeffs.size() - 1u; ++k )
  {
    coeffs[k + 1u] = coeffs[k] * ( nmink - k ) / ( k + 1u );
  }

  if ( (bool)sigp && (bool)sigt )
  {
    for ( auto k = 0u; k < maxk + 1u; ++k )
    {
      /* cold */
      auto pvalue_c = (*sigp)[k << 1u];
      auto pvalue_h = (*sigp)[(k << 1u) + 1u];
      for ( auto j = 1u; j <= k; ++j )
      {
        pvalue_c += (*sigp)[(k - j) << 1u] * coeffs[j];
        pvalue_h += (*sigp)[((k - j) << 1u) + 1u] * coeffs[j];
      }

      if ( ( (*sigt)[k << 1u] != pvalue_c ) || ( (*sigt)[(k << 1u) + 1u] != pvalue_h ) ) { return false; }
    }
  }

  return true;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
