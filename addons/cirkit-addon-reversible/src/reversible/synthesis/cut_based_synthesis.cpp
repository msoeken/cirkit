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

#include "cut_based_synthesis.hpp"

#include <map>
#include <queue>
#include <stack>
#include <unordered_set>

#include <boost/assign/std/vector.hpp>
#include <boost/format.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/optional.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/adaptors.hpp>

#include <core/utils/bdd_utils.hpp>
#include <core/utils/bitset_utils.hpp>
#include <core/utils/graph_utils.hpp>
#include <core/utils/range_utils.hpp>
#include <core/utils/terminal.hpp>
#include <core/utils/timer.hpp>
#include <classical/functions/fanout_free_regions.hpp>
#include <classical/functions/simulate_aig.hpp>
#include <classical/optimization/exorcism_minimization.hpp>
#include <classical/utils/cut_enumeration.hpp>
#include <classical/utils/aig_utils.hpp>
#include <reversible/rcbdd.hpp>
#include <reversible/truth_table.hpp>
#include <reversible/functions/add_gates.hpp>
#include <reversible/functions/add_line_to_circuit.hpp>
#include <reversible/functions/extend_pla.hpp>
#include <reversible/functions/truth_table_from_bitset.hpp>
#include <reversible/io/print_circuit.hpp>
#include <reversible/io/write_pla.hpp>
#include <reversible/synthesis/embed_bdd.hpp>
#include <reversible/synthesis/embed_pla.hpp>
#include <reversible/synthesis/exact_synthesis.hpp>
#include <reversible/synthesis/rcbdd_synthesis.hpp>
#include <reversible/synthesis/symbolic_transformation_based_synthesis.hpp>

#define timer timer_class
#include <boost/progress.hpp>
#undef timer

#include <cuddObj.hh>

using namespace boost::assign;

namespace cirkit
{

/******************************************************************************
 * Macros                                                                     *
 ******************************************************************************/

#define L(x) { if ( verbose ) { std::cout << x << std::endl; } }

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

using aig_node_vec_t    = std::vector<aig_node>;
using ffrs_t            = std::map<aig_node, aig_node_vec_t>;
using ffrs_region_t     = std::tuple<aig_node_vec_t, aig_node_vec_t, aig_node_vec_t>;
using ffrs_region_vec_t = std::vector<ffrs_region_t>;

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

std::map<aig_node, std::stack<aig_node>> get_inputs_to_ffr_map( const aig_graph& aig,
                                                                const ffrs_t& ffrs,
                                                                const aig_node_vec_t& ffrs_topsort )
{
  using boost::adaptors::reversed;

  std::map<aig_node, std::stack<aig_node>> ffr_inputs_map;

  for ( const auto& output : aig_info( aig ).outputs )
  {
    ffr_inputs_map[output.first.node].push( output.first.node );
  }

  for ( const auto& v : ffrs_topsort | reversed )
  {
    if ( ffrs.at( v ).size() == 1u && ffrs.at( v ).front() == v ) { continue; }

    for ( const auto& input : ffrs.at( v ) )
    {
      ffr_inputs_map[input].push( v );
    }
  }

  return std::move( ffr_inputs_map );
}

aig_node_vec_t merge_copy( const aig_node_vec_t& a,
                           const aig_node_vec_t& b,
                           aig_node_vec_t& outputs,
                           aig_node_vec_t& internal )
{
  //std::cout << "[i] merge " << any_join( a, ", " ) << " + " << any_join( b, ", " ) << "  outputs: " << any_join( outputs, ", " ) << ", internal: " << any_join( internal, ", " ) << std::endl;
  aig_node_vec_t ret;

  for ( const auto& elem : a )
  {
    if ( boost::find( internal, elem ) == internal.end() )
    {
      ret += elem;
    }
  }

  for ( const auto& elem : b )
  {
    const auto it = boost::find( outputs, elem );
    if ( it != outputs.end() )
    {
      outputs.erase( it );
      internal += elem;
    }

    if ( boost::find( ret, elem ) == ret.end() && boost::find( internal, elem ) == internal.end() )
    {
      ret += elem;
    }
  }

  return ret;
}

ffrs_region_vec_t synthesizable_regions( const ffrs_t& ffrs, const aig_node_vec_t& ffrs_topsort,
                                         std::map<aig_node, std::stack<aig_node>>& ffr_inputs_map,
                                         unsigned threshold, const std::vector<aig_node>& aig_outputs,
                                         bool verbose )
{
  ffrs_region_vec_t regions;

  auto i = 0u;

  while ( i < ffrs_topsort.size() )
  {
    std::vector<aig_node> inputs, outputs, preserve_inputs, internal;

    while ( true )
    {
      const auto& ffr        = ffrs_topsort.at( i );
      const auto& ffr_inputs = ffrs.at( ffr );

      /* buffer */
      if ( ffr_inputs.size() == 1u && ffr == ffr_inputs.front() )
      {
        break;
      }

      if ( inputs.size() + ffr_inputs.size() + outputs.size() + 1u > threshold ) { break; }

      //L( boost::format( "[i] merge %d(%s)" ) % ffr % any_join( ffr_inputs, ", " ) );

      auto inputs_copy = merge_copy( inputs, ffr_inputs, outputs, internal );

      /* break loop */
      //if ( ( outputs.size() > 0 ) && ( inputs_copy.size() + outputs.size() + 1u > threshold ) ) { break; }
      //std::cout << inputs_copy.size() << std::endl;
      //if ( inputs_copy.size() + outputs.size() + 1u > threshold ) { break; }

      /* update inputs and outputs */
      inputs = inputs_copy;
      outputs += ffr;

      //L( "[i] inputs: " << any_join( inputs, ", " ) << ", outputs: " << any_join( outputs, ", " ) );

      /* remove dependent ffrs from stacks */
      for ( const auto& input : ffr_inputs )
      {
        assert( ffr_inputs_map.at( input ).top() == ffr );
        ffr_inputs_map.at( input ).pop();
      }

      ++i;

      if ( i == ffrs_topsort.size() ) { break; }
    }

    if ( inputs.empty() )
    {
      ++i; continue;
    }

    /* check for shared inputs */
    for ( const auto& input : inputs )
    {
      if ( !ffr_inputs_map.at( input ).empty() )
      {
        preserve_inputs += input;
      }
    }

    /* do we still need some of the internals? */
    for ( const auto& input : internal )
    {
      const auto it = ffr_inputs_map.find( input );
      if ( ( it != ffr_inputs_map.end() && !it->second.empty() ) ||
           boost::find( aig_outputs, input ) != aig_outputs.end() )
      {
        outputs += input;
      }
    }

    L( boost::format( "[i] found region (i: %d, #inputs: %d [%s], #outputs: %d [%s])" ) % i % inputs.size() % any_join( inputs, ", " ) % outputs.size() % any_join( outputs, ", " ) );

    regions += std::make_tuple( inputs, outputs, preserve_inputs );
  }

  return std::move( regions );
}

std::vector<BDD> simulate_synthesizable_region( const aig_graph& aig,
                                                const ffrs_region_t& region,
                                                std::map<aig_node, BDD>& node_to_input,
                                                Cudd& mgr )
{
  for ( const auto& input : std::get<0>( region ) )
  {
    node_to_input.insert( {input, mgr.bddVar()} );
  }

  bdd_simulator bdd_sim( mgr );
  aig_partial_node_assignment_simulator<BDD> sim( bdd_sim, node_to_input, mgr.bddZero() );

  aig_node_color_map      colors;
  std::map<aig_node, BDD> node_values;
  std::vector<BDD>        bdd_outputs;

  for ( const auto& output : std::get<1>( region ) )
  {
    bdd_outputs += simulate_aig_node( aig, output, sim, colors, node_values );
  }

  return bdd_outputs;
}

binary_truth_table pla_from_bdd( const Cudd& mgr, const std::vector<BDD>& outputs )
{
  /* collect entries */
  std::map<binary_truth_table::cube_type, binary_truth_table::cube_type> entries;

  for ( const auto& f : index( outputs ) )
  {
    DdGen *gen;
    int *cube;
    CUDD_VALUE_TYPE value;
    Cudd_ForeachCube( mgr.getManager(), f.value.getNode(), gen, cube, value )
    {
      binary_truth_table::cube_type in( mgr.ReadSize(), boost::optional<bool>() );

      for ( auto i = 0; i < mgr.ReadSize(); ++i )
      {
        if ( cube[i] == 0 )
        {
          in[i] = false;
        }
        else if ( cube[i] == 1 )
        {
          in[i] = true;
        }
      }

      const auto it = entries.find( in );

      if ( it == entries.end() )
      {
        entries.insert( {in, binary_truth_table::cube_type( outputs.size(), false )} );
      }

      entries[in][f.index] = true;
    }
  }

  /* create table from entries */
  binary_truth_table table;

  for ( const auto& p : entries )
  {
    table.add_entry( p.first, p.second );
  }

  return table;
}

rcbdd embed_with_pla( const bdd_function_t& bdd, bool verbose )
{
  if ( verbose )
  {
    std::cout << "[i] create PLA from BDDs" << std::endl;
  }
  auto pla = pla_from_bdd( bdd.first, bdd.second );
  binary_truth_table extended;

  if ( verbose )
  {
    std::cout << "[i] computing DSOP" << std::endl;
  }
  extend_pla( pla, extended );

  write_pla( extended, "/tmp/extended.pla" );

  if ( verbose )
  {
    std::cout << "[i] embedding DSOP" << std::endl;
  }
  rcbdd cf;
  embed_pla( cf, "/tmp/extended.pla" );

  return cf;
}

rcbdd embed_with_bdd( const bdd_function_t& bdd, bool verbose )
{
  if ( verbose )
  {
    std::cout << "[i]   embedding" << std::endl;
  }

  rcbdd cf;
  embed_bdd( cf, bdd );

  return cf;
}

bool inputs_equal( const std::vector<aig_node>& a, const std::vector<aig_node>& b )
{
  if ( a.size() != b.size() ) { return false; }

  for ( const auto& e : a )
  {
    if ( boost::find( b, e ) == b.end() )
    {
      return false;
    }
  }

  return true;
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

bool cut_based_synthesis( circuit& circ, const aig_graph& aig,
                          const properties::ptr& settings,
                          const properties::ptr& statistics )
{
  /* settings */
  const auto var_threshold      = get( settings, "var_threshold",      20u );
  const auto embedding          = get( settings, "embedding",          0u ); /* 0u: BDD based, 1u: PLA based */
  const auto synthesis          = get( settings, "synthesis",          0u ); /* 0u: TBS (BDD), 1u: TBS (SAT), 2u: DBS */
  const auto store_intermediate = get( settings, "store_intermediate", false );
  const auto progress           = get( settings, "progress",           false );
  const auto verbose            = get( settings, "verbose",            false );

  std::vector<bdd_function_t> si_bdds;
  std::vector<rcbdd>          si_rcbdds;
  std::vector<circuit>        si_circuits;

  /* timer */
  properties_timer t( statistics );

  const auto& info = aig_info( aig );

  /******************************************
   * Some pre-processing                    *
   ******************************************/

  /* compute topological order of vertices */
  aig_node_vec_t topsort( boost::num_vertices( aig ) );
  boost::topological_sort( aig, topsort.begin() );

  /*******************************************
   * Partitioning into fanout free regions   *
   *******************************************/

  L("[i] start FFR computation");

  /* compute outputs */
  std::vector<aig_node> aig_outputs;
  for ( const auto& output : info.outputs )
  {
    aig_outputs += output.first.node;
  }

  /* compute fanout free regions */
  auto ffr_settings = std::make_shared<properties>();
  ffr_settings->set( "verbose",    verbose );
  ffr_settings->set( "max_inputs", var_threshold - 1u );
  ffr_settings->set( "outputs",    aig_outputs );
  const auto ffrs = fanout_free_regions_bfs( aig, ffr_settings );

  /* sort FFRs in topoplogical order */
  const auto ffrs_topsort = topological_sort_ffrs<aig_graph>( ffrs, topsort );

  /* map inputs to FFRs */
  auto ffr_inputs_map = get_inputs_to_ffr_map( aig, ffrs, ffrs_topsort );

  /*******************************************
   * prepare circuit                         *
   *******************************************/

  std::map<aig_node, unsigned> node_to_line;
  for ( const auto& input : info.inputs )
  {
    node_to_line.insert( {input, circ.lines()} );
    add_line_to_circuit( circ, info.node_names.at( input ), boost::str( boost::format( "garb%d" ) % circ.lines() ), constant(), true );
  }

  if ( verbose )
  {
    for ( const auto& pair : node_to_line )
    {
      std::cout << "[i] node " << pair.first << " maps to line " << pair.second << std::endl;
    }
  }

  /*******************************************
   * collect FFRs according to threshold     *
   *******************************************/

  /* merge FFRs according to threshold and compute I/Os for each region */
  const auto synth_regions = synthesizable_regions( ffrs, ffrs_topsort, ffr_inputs_map, var_threshold, aig_outputs, verbose );

  /*******************************************
   * synthesize regions                      *
   *******************************************/
  null_stream ns;
  std::ostream null_out( &ns );
  boost::progress_display show_progress( synth_regions.size(), progress ? std::cout : null_out );

  for ( const auto& region : index( synth_regions ) )
  {
    const auto& inputs          = std::get<0>( region.value );
    const auto& outputs         = std::get<1>( region.value );
    const auto& preserve_inputs = std::get<2>( region.value );

    //boost::sort( inputs );
    //boost::sort( preserve_inputs );

    ++show_progress;

    L( boost::format( "[i] synth region %d/%d: {%s} |-> {%s} (preserve %s)" ) % ( region.index + 1u ) % synth_regions.size() \
                                                                              % any_join( inputs, ", " )  \
                                                                              % any_join( outputs, ", " ) \
                                                                              % any_join( preserve_inputs, ", " ) );

    for ( const auto& input : inputs )
    {
      assert( boost::find( outputs, input ) == outputs.end() );
    }

    for ( const auto& input : preserve_inputs )
    {
      assert( boost::find( inputs, input ) != inputs.end() );
    }

    /* synthesize BDDs via simulation */
    Cudd mgr;
    std::map<aig_node, BDD> node_to_input;

    auto bdd_outputs = simulate_synthesizable_region( aig, region.value, node_to_input, mgr );

    if ( inputs_equal( inputs, preserve_inputs ) )
    {
      L( "[i]   special case: all inputs need to be preserved -> Bennett embedding" );

      /* add required lines (one for each output) */
      auto target = circ.lines();
      for ( const auto& output : outputs )
      {
        assert( boost::find( inputs, output ) == inputs.end() );
        node_to_line[output] = circ.lines();
        add_line_to_circuit( circ, "0", boost::str( boost::format( "garb%d" ) % circ.lines() ), false, true );
      }

      /* we synthesize the subcircuit directly into circ */
      const auto cube_function = [&]( const cube_t& cube ) {
        gate::control_container controls;

        foreach_bit( cube.second, [&]( unsigned pos ) {
            if ( node_to_line.find( inputs[pos] ) == node_to_line.end() )
            {
              std::cout << inputs[pos] << " is not in node_to_line" << std::endl;
              assert( false );
            }
            controls += make_var( node_to_line[inputs[pos]], cube.first.test( pos ) );
          } );

        append_toffoli( circ, controls, target );
      };

      const auto em_settings = std::make_shared<properties>();
      em_settings->set( "on_cube", cube_function_t( cube_function ) );

      for ( const auto& f : bdd_outputs )
      {
        exorcism_minimization( mgr.getManager(), f.getNode(), em_settings );
        ++target;
      }
    }
    else
    {
      /* preserve inputs */
      for ( const auto& input : std::get<2>( region.value ) )
      {
        bdd_outputs += node_to_input.at( input );
      }

      /* embedding */
      bdd_function_t bdd = {mgr, bdd_outputs};
      rcbdd cf;
      if ( embedding == 0u )
      {
        cf = embed_with_bdd( bdd, verbose );
      }
      else
      {
        cf = embed_with_pla( bdd, verbose );
      }

      if ( store_intermediate )
      {
        si_bdds.push_back( bdd );
        si_rcbdds.push_back( cf );
      }

      /* add constant lines of embedding */
      const auto additional = cf.num_vars() - inputs.size();
      const auto start_index = circ.lines();
      ntimes( additional, [&]() {
          add_line_to_circuit( circ, "0", boost::str( boost::format( "garb%d" ) % circ.lines() ), false, true );
        } );

      /* circuit map */
      std::vector<unsigned> circuit_line_map( cf.num_vars() );
      for ( auto c = 0u; c < additional; ++c )
      {
        circuit_line_map[c] = start_index + c;
      }
      for ( auto c = 0u; c < inputs.size(); ++c )
      {
        circuit_line_map[additional + c] = node_to_line[inputs[c]];
      }

      if ( verbose )
      {
        for ( auto i = 0u; i < circuit_line_map.size(); ++i )
        {
          //std::cout << "[i] circuit_line_map[" << i << "] = " << circuit_line_map[i] << std::endl;
        }
      }

      /* update node lines */
      std::map<aig_node, unsigned> update_node_to_line;
      auto index = 0u;
      for ( const auto& output : outputs )
      {
        const auto line = ( index < additional ) ? ( start_index + index ) : ( node_to_line[inputs[index - additional]] );
        update_node_to_line[output] = line;
        ++index;
      }

      for ( const auto& input : preserve_inputs )
      {
        const auto line = ( index < additional ) ? ( start_index + index ) : ( node_to_line[inputs[index - additional]] );
        update_node_to_line[input] = line;
        ++index;
      }
      for ( const auto& p : update_node_to_line )
      {
        node_to_line[p.first] = p.second;
      }

      L( boost::format( "[i]   synthesizing (n: %d)" ) % cf.num_vars() );

      circuit part_circ;
      auto synth_settings = std::make_shared<properties>();
      synth_settings->set( "verbose", /* verbose */ false );

      if ( synthesis == 0u )
      {
        symbolic_transformation_based_synthesis( part_circ, cf, synth_settings );
      }
      else if ( synthesis == 1u )
      {
        symbolic_transformation_based_synthesis_sat( part_circ, cf, synth_settings );
      }
      else /*if ( synthesis == 2u )*/
      {
        synth_settings->set( "esopmin", dd_based_exorcism_minimization_func() );
        rcbdd_synthesis( part_circ, cf, synth_settings );
      }

      if ( store_intermediate )
      {
        si_circuits.push_back( part_circ );
      }

      /* copy part_circ to real circuit */
      for ( const auto& g : part_circ )
      {
        gate::control_container controls;
        for ( const auto& c : g.controls() )
        {
          controls += make_var( circuit_line_map[c.line()], c.polarity() );
        }
        append_toffoli( circ, controls, circuit_line_map[g.targets().front()] );
      }
    }
  }


  /*******************************************
   * create outputs                          *
   *******************************************/
  std::map<unsigned, aig_function> line_to_output;
  for ( const auto& output : info.outputs )
  {
    L( "[i] add output " << output.second );
    if ( output.first.node == 0u )
    {
      L( "[i] output is constant" );
      add_line_to_circuit( circ, output.first.complemented ? "1" : "0", output.second, output.first.complemented, false );
      continue;
    }

    if ( node_to_line.find( output.first.node ) == node_to_line.end() )
    {
      L( "[i] cannot find line for output " << output.first.node << std::endl );
      assert( false );
    }
    const auto line = node_to_line.at( output.first.node );

    if ( !circ.garbage()[line] )
    {
      L( "[i] output has been used before, copy it" );
      auto diff_compl = line_to_output[line].complemented != output.first.complemented;
      add_line_to_circuit( circ, diff_compl ? "1" : "0", output.second, diff_compl, false );
      append_cnot( circ, line, circ.lines() - 1u );
    }
    else
    {
      L( "[i] add output to line " << line );
      if ( output.first.complemented )
      {
        L( "[i] output is complemented" );
        append_not( circ, line );
      }

      auto garbage = circ.garbage();
      auto outputs = circ.outputs();

      garbage[line] = false;
      outputs[line] = output.second;

      circ.set_garbage( garbage );
      circ.set_outputs( outputs );

      line_to_output[line] = output.first;
    }
  }

  L( "[i] total gates: " << circ.num_gates() );
  L( "[i] total lines: " << circ.lines() );

  if ( store_intermediate )
  {
    set( statistics, "bdds",     si_bdds );
    set( statistics, "rcbdds",   si_rcbdds );
    set( statistics, "circuits", si_circuits );
  }

  return true;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
