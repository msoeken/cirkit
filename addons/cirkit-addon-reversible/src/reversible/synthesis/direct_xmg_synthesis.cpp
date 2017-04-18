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

#include "direct_xmg_synthesis.hpp"

#include <fstream>
#include <queue>
#include <stack>
#include <vector>

#include <boost/dynamic_bitset.hpp>
#include <boost/filesystem.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>

#include <core/utils/range_utils.hpp>
#include <core/utils/string_utils.hpp>
#include <core/utils/timer.hpp>
#include <classical/utils/truth_table_utils.hpp>
#include <classical/xmg/xmg_coi.hpp>
#include <classical/xmg/xmg_mffc.hpp>
#include <classical/xmg/xmg_simulate.hpp>
#include <classical/xmg/xmg_utils.hpp>
#include <reversible/target_tags.hpp>
#include <reversible/functions/add_circuit.hpp>
#include <reversible/functions/add_gates.hpp>
#include <reversible/functions/add_line_to_circuit.hpp>
#include <reversible/functions/circuit_from_string.hpp>
#include <reversible/functions/clear_circuit.hpp>
#include <reversible/functions/reverse_circuit.hpp>
#include <reversible/functions/truth_table_from_bitset.hpp>
#include <reversible/io/print_circuit.hpp>
#include <reversible/synthesis/exact_synthesis.hpp>
#include <reversible/synthesis/exact_toffoli_synthesis.hpp>
#include <reversible/synthesis/transformation_based_synthesis.hpp>

#define L(x) if ( verbose ) { std::cout << x << std::endl; }

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

class dxs_exact_manager
{
public:
  dxs_exact_manager( xmg_graph& xmg, bool verbose, unsigned var_threshold )
    : xmg( xmg ),
      verbose( verbose ),
      var_threshold( var_threshold ),
      skip_list( xmg.size() )
  {
    xmg.compute_parents();
  }

  void run()
  {
    L( "[i] compute MFFCs" );
    mffcs = xmg_mffcs( xmg );

    compute_truth_tables();
    compute_circuits();
  }

  bool has_network( xmg_node n ) const
  {
    return mffc_specs.find( n ) != mffc_specs.end();
  }

  const circuit& get_network( xmg_node n ) const
  {
    return *mffc_circs.at( tt_to_hex( mffc_specs.at( n ) ) );
  }

  const std::vector<xmg_node>& get_leafs( xmg_node n ) const
  {
    return mffcs.at( n );
  }

  bool must_skip( xmg_node n ) const
  {
    return skip_list.test( n );
  }

  std::vector<unsigned> get_leaf_fanouts( xmg_node n ) const
  {
    const auto& leafs = mffcs.at( n );
    auto cone = xmg_mffc_cone( xmg, n, leafs );
    boost::push_back( cone, leafs );

    std::vector<unsigned> fanouts;

    for ( auto l : leafs )
    {
      auto fanout = 0;

      for ( auto p : xmg.parents( l ) )
      {
        if ( boost::find( cone, p ) != cone.end() )
        {
          ++fanout;
        }
      }

      assert( fanout );
      fanouts.push_back( fanout );
    }

    return fanouts;
  }

  const std::map<xmg_node, std::vector<xmg_node>>& get_mffcs() const
  {
    return mffcs;
  }

private:
  void compute_truth_tables()
  {
    std::vector<std::string> blacklist( {"abba0330", "44504040"} );

    for ( const auto& p : mffcs )
    {
      auto num_vars = p.second.size();
      if ( boost::find( p.second, 0 ) != p.second.end() ) /* leafs contain constant */
      {
        --num_vars;
      }

      /* too many vars */
      if ( num_vars < 2 || num_vars > var_threshold ) { continue; }

      const auto size = xmg_mffc_size( xmg, p.first, p.second );

      /* too few nodes */
      if ( size == 1 ) { continue; }

      std::map<xmg_node, tt> node_map;
      auto i = 0u;
      for ( auto leaf : p.second )
      {
        if ( leaf == 0 ) { continue; }

        auto leaf_spec = tt_nth_var( num_vars - 1 - i++ );
        if ( num_vars > 6 )
        {
          tt_extend( leaf_spec, num_vars );
        }
        node_map[leaf] = leaf_spec;
      }

      xmg_tt_simulator sim;
      xmg_partial_node_assignment_simulator<tt> psim( sim, node_map, tt_const0() );

      auto spec = simulate_xmg_node( xmg, p.first, psim );
      if ( num_vars < 6 )
      {
        tt_shrink( spec, num_vars );
      }

      /* even permutation? */
      if ( spec.count() % 2 != 0 ) { continue; }

      /* blacklisted? */
      const auto spec_str = tt_to_hex( spec );
      if ( boost::find( blacklist, spec_str ) != blacklist.end() ) { continue; }

      mffc_specs[p.first] = spec;
      mffc_circs[spec_str] = boost::none;

      /* update skip list */
      for ( auto node : xmg_mffc_cone( xmg, p.first, p.second ) )
      {
        skip_list.set( node );
      }
      skip_list.reset( p.first );

      L( boost::format( "[i] MFFC for %d with leafs %s and size %d: %s" ) % p.first % any_join( p.second, ", " ) % size % tt_to_hex( spec ) );
    }
  }

  void read_circuits_from_file()
  {
    if ( const auto* path = std::getenv( "CIRKIT_HOME" ) )
    {
      const auto filename = boost::str( boost::format( "%s/dxsmin.txt" ) % path );
      if ( boost::filesystem::exists( filename ) )
      {
        foreach_line_in_file( filename, [this]( const std::string& line ) {
            const auto split = line.find( ' ' );

            mffc_circs[line.substr( 0u, split )] = circuit_from_string( line.substr( split + 1 ) );

            return true;
          } );
      }
    }
  }

  void write_circuits_to_file()
  {
    if ( const auto* path = std::getenv( "CIRKIT_HOME" ) )
    {
      const auto filename = boost::str( boost::format( "%s/dxsmin.txt" ) % path );
      std::ofstream os( filename.c_str(), std::ofstream::out );

      for ( const auto& p : mffc_circs )
      {
        if ( !(bool)p.second ) { continue; }

        os << p.first << " " << circuit_to_string( *p.second ) << std::endl;
      }
    }
  }

  void compute_circuits()
  {
    //read_circuits_from_file();

    for ( const auto& p : mffc_specs )
    {
      const auto spec_str = tt_to_hex( p.second );

      /* circuit already computed */
      if ( (bool)mffc_circs[spec_str] ) { continue; }

      L( boost::format( "[i] compute optimum circuit for %s" ) % spec_str );

      const auto rev_spec = truth_table_from_bitset_bennett( p.second );
      circuit circ;
      auto es_settings = std::make_shared<properties>();
      es_settings->set( "negative", true );
      es_settings->set( "only_toffoli", true );
      es_settings->set( "verbose", false );
      //exact_synthesis( circ, rev_spec, es_settings );
      transformation_based_synthesis( circ, rev_spec, es_settings );

      mffc_circs[spec_str] = circ;

      //write_circuits_to_file();
    }
  }

private:
  xmg_graph& xmg;
  bool verbose;

  unsigned var_threshold = 15u;

  std::map<xmg_node, std::vector<xmg_node>>       mffcs;
  std::map<xmg_node, tt>                          mffc_specs;
  std::map<std::string, boost::optional<circuit>> mffc_circs;

  boost::dynamic_bitset<>                         skip_list; /* nodes to skip in mapping */
};

class direct_xmg_synthesis_manager
{
public:
  direct_xmg_synthesis_manager( circuit& circ, xmg_graph& xmg, const properties::ptr& settings )
    : circ( circ ),
      xmg( xmg ),
      node_to_line( xmg.size() ),
      line_to_node( xmg.inputs().size() ),
      is_garbage( xmg.inputs().size() ),

      inplace( get( settings, "inplace", inplace ) ),
      var_threshold( get( settings, "var_threshold", 0u ) ),
      garbage_name( get( settings, "garbage_name", garbage_name ) ),
      verbose( get( settings, "verbose", verbose ) ),

      esmgr( xmg, verbose, var_threshold )
  {
    clear_circuit( circ );

    /* initialize reference counting */
    xmg.init_refs();
    xmg.inc_output_refs();

    if ( var_threshold )
    {
      esmgr.run();
    }

    /* initialize reference counting for uncomputing */
    if ( !inplace )
    {
      is_output = xmg_output_mask( xmg );
      xmg.compute_fanout();
      unccounter.resize( xmg.size() );
      for ( auto node : xmg.nodes() )
      {
        unccounter[node] = xmg.fanout_count( node );
      }

      /* adjust unccounter */
      for ( const auto& p : esmgr.get_mffcs() )
      {
        if ( !esmgr.has_network( p.first ) ) { continue; }
        for ( auto l : p.second )
        {
          unccounter[l] = 0; //1;
        }
      }
      for ( const auto& p : esmgr.get_mffcs() )
      {
        if ( !esmgr.has_network( p.first ) ) { continue; }
        for ( auto l : p.second )
        {
          unccounter[l] = xmg.fanout_count( l );
        }
      }
    }
  }

  bool run()
  {
    add_inputs();

    //for ( auto node : xmg.topological_nodes() )
    for ( auto node : xmg_coi_topological_nodes( xmg ) )
    {
      if ( xmg.is_input( node ) ) { continue; }
      if ( esmgr.must_skip( node ) ) { continue; }

      const auto line = add_node( node );

      /* decrement reference counters and update line for node */
      node_to_line[node] = line;
      line_to_node[line] = node;
      L( boost::format( "[i] node %d -> line %d" ) % node % node_to_line[node] );

      /* trigger uncomputing */
      if ( inplace ) /* uncompute as early as possible */
      {
        foreach_child( node, [this]( xmg_node child ) {
            xmg.dec_ref( child );
            if ( xmg.get_ref( child ) == 0 && !xmg.is_input( child ) )
            {
              if ( can_uncompute( child ) )
              {
                L( boost::format( "[i] uncompute node %d" ) % child );
                add_node_uncompute( child );
              }
              else
              {
                L( boost::format( "[i] cannot uncompute node %d" ) % child );
                is_garbage[node_to_line[child]] = true;
              }
            }
            return true;
          } );
      }
      else /* defer uncomputing */
      {
        /* if node has output but no other fanout */
        if ( is_output[node] && xmg.fanout_count( node ) == 0 )
        {
          L( "[i] start deferred uncomputing from node " << node );
          foreach_child( node, [this]( xmg_node child ) {
              try_uncompute_defer( child );
              return true;
            } );
        }
      }
    }

    add_outputs();

    return true;
  }

private:
  template<typename Fn>
  bool foreach_child( xmg_node n, Fn&& f )
  {
    if ( esmgr.has_network( n ) )
    {
      for ( auto leaf : esmgr.get_leafs( n ) )
      {
        if ( !f( leaf ) ) { return false; }
      }
    }
    else
    {
      for ( const auto& child : xmg.children( n ) )
      {
        if ( !f( child.node ) ) { return false; }
      }
    }

    return true;
  }

  unsigned request_constant()
  {
    if ( constants.empty() )
    {
      is_garbage.push_back( false );
      line_to_node.push_back( 0 );
      return add_line_to_circuit( circ, "0", garbage_name, false, true ); /* garbage attributes are temporary */
    }
    else
    {
      const auto c = constants.top();
      constants.pop();
      return c;
    }
  }

  bool can_uncompute( xmg_node node )
  {
    /* node must still be available */
    if ( line_to_node[node_to_line[node]] != node )
    {
      return false;
    }

    /* children must still be available */
    if ( !foreach_child( node, [this]( xmg_node child ) {
        return ( child == 0 || line_to_node[node_to_line[child]] == child );
        } ) )
    {
      return false;
    }

    return true;
  }

  bool can_uncompute_defer( xmg_node node )
  {
    if ( xmg.is_input( node ) || is_output[node] )
    {
      return false;
    }

    if ( unccounter[node] > 1 )
    {
      unccounter[node]--;
      return false;
    }

    assert( unccounter[node] == 1 );

    return true;
  }

  void try_uncompute_defer( xmg_node node )
  {
    assert( !esmgr.must_skip( node ) );

    if ( !can_uncompute_defer( node ) )
    {
      return;
    }

    L( boost::format( "[i] uncompute node %d" ) % node );
    add_node_uncompute( node );

    foreach_child( node, [this]( xmg_node child ) {
        try_uncompute_defer( child );
        return true;
      } );
  }

  unsigned add_node( xmg_node node )
  {
    const auto children = xmg.children( node );

    if ( esmgr.has_network( node ) )
    {
      L( boost::format( "[i] node %d is computed network" ) % node );

      return add_computed_network( node );
    }
    else if ( xmg.is_xor( node ) ) /* XOR */
    {
      if ( inplace && xmg.get_ref( children[0u].node ) == 1u )
      {
        L( boost::format( "[i] node %d is XOR of %d and %d, can be done in place, storing to %d" ) % node % children[0u].node % children[1u].node % children[0u].node );

        /* set a <- a XOR b */
        return add_xor_inplace( children[0u], children[1u] );
      }
      else if ( inplace && xmg.get_ref( children[1u].node ) == 1u )
      {
        L( boost::format( "[i] node %d is XOR of %d and %d, can be done in place, storing to %d" ) % node % children[0u].node % children[1u].node % children[1u].node );

        /* set b <- a XOR b */
        return add_xor_inplace( children[1u], children[0u] );
      }
      else
      {
        L( boost::format( "[i] node %d is XOR of %d and %d" ) % node % children[0u].node % children[1u].node );

        return add_xor( children[0u], children[1u] );
      }
    }
    else /* MAJ, AND, OR */
    {
      if ( children[0u].node == 0u )
      {
        if ( children[0u].complemented ) /* OR */
        {
          L( boost::format( "[i] node %d is OR of %d and %d" ) % node % children[1u].node % children[2u].node );

          return add_or( children[1u], children[2u] );
        }
        else /* AND */
        {
          L( boost::format( "[i] node %d is AND of %d and %d" ) % node % children[1u].node % children[2u].node );

          return add_and( children[1u], children[2u] );
        }
      }
      else /* MAJ */
      {
        if ( inplace &&
             xmg.get_ref( children[0u].node ) == 1u &&
             xmg.get_ref( children[1u].node ) == 1u &&
             xmg.get_ref( children[2u].node ) == 1u )
        {
          L( boost::format( "[i] node %d is MAJ of %d and %d and %d, can be done in place" ) % node % children[0u].node % children[1u].node % children[2u].node );

          return add_maj_inplace( children[0u], children[1u], children[2u] );
        }
        else
        {
          L( boost::format( "[i] node %d is MAJ of %d and %d and %d" ) % node % children[0u].node % children[1u].node % children[2u].node );

          return add_maj( children[0u], children[1u], children[2u] );
        }
      }
    }

    assert( false );
  }

  void add_node_uncompute( xmg_node node )
  {
    const auto children = xmg.children( node );

    if ( esmgr.has_network( node ) )
    {
      add_computed_network_uncompute( node );
    }
    else if ( xmg.is_xor( node ) ) /* XOR */
    {
      add_xor_uncompute( children[0u], children[1u], node );
    }
    else /* MAJ, AND, OR */
    {
      if ( children[0u].node == 0u )
      {
        if ( children[0u].complemented ) /* OR */
        {
          add_or_uncompute( children[1u], children[2u], node );
        }
        else /* AND */
        {
          add_and_uncompute( children[1u], children[2u], node );
        }
      }
      else /* MAJ */
      {
        add_maj_uncompute( children[0u], children[1u], children[2u], node );
      }
    }
  }

  void add_inputs()
  {
    for ( const auto& input : xmg.inputs() )
    {
      const auto line = add_line_to_circuit( circ, input.second, garbage_name, constant(), true );
      node_to_line[input.first] = line;
      line_to_node[line] = input.first;

      L( boost::format( "[i] input %d -> line %d" ) % input.first % line );
    }
  }

  void add_outputs()
  {
    auto garbage = circ.garbage();
    auto outputs = circ.outputs();

    /* get output bitset */
    boost::dynamic_bitset<> boutputs( circ.lines() );
    for ( const auto& output : xmg.outputs() )
    {
      boutputs.set( node_to_line[output.first.node] );
    }

    /* fix non-output lines */
    if ( !inplace )
    {
      for ( auto i = 0u; i < circ.lines(); ++i )
      {
        if ( boutputs[i] )
        {
          continue;
        }
        else if ( is_garbage[i] )
        {
          garbage[i] = true;
          outputs[i] = garbage_name;
        }
        else if ( line_to_node[i] == 0 ) /* is constant */
        {
          garbage[i] = true; /* but not really */
          outputs[i] = "0";
        }
        else
        {
          const auto node = line_to_node[i];
          if ( !xmg.is_input( node ) )
          {
            assert( unccounter[node] );
            add_node_uncompute( node );
            garbage[i] = true;
            outputs[i] = "0";
          }
          else
          {
            garbage[i] = true; /* but not really */
            outputs[i] = xmg.input_name( line_to_node[i] );
          }
        }
      }
    }

    std::vector<unsigned> output_to_line;

    for ( const auto& output : xmg.outputs() )
    {
      assert( xmg.get_ref( output.first.node ) );

      auto l = node_to_line[output.first.node];
      if ( garbage[l] ) /* node is used first time */
      {
        L( boost::format( "[i] add output %d to line %d" ) % output_to_line.size() % l );
        garbage[l] = false;
        outputs[l] = output.second;
      }
      else
      {
        auto old_line = l;
        l = request_constant();
        L( boost::format( "[i] add output %d to line %d" ) % output_to_line.size() % l );
        append_cnot( circ, old_line, l );
        garbage.push_back( false );
        outputs.push_back( output.second );
      }

      output_to_line.push_back( l );
    }

    /* second path for complemented outputs */
    for ( const auto& output : index( xmg.outputs() ) )
    {
      if ( output.value.first.complemented )
      {
        L( boost::format( "[i] invert output %d" ) % output.index );
        append_not( circ, output_to_line[output.index] );
      }
    }

    circ.set_garbage( garbage );
    circ.set_outputs( outputs );
  }

  unsigned add_xor_inplace( const xmg_function& dest, const xmg_function& src )
  {
    assert( dest.node && src.node );
    assert( !dest.complemented );
    assert( !src.complemented );

    assert( xmg.get_ref( dest.node ) );
    assert( xmg.get_ref( src.node ) );

    append_cnot( circ, node_to_line[src.node], node_to_line[dest.node] );

    return node_to_line[dest.node];
  }

  unsigned add_xor( const xmg_function& op1, const xmg_function& op2 )
  {
    assert( op1.node && op2.node );
    assert( !op1.complemented );
    assert( !op2.complemented );

    assert( xmg.get_ref( op1.node ) );
    assert( xmg.get_ref( op2.node ) );

    const auto target = request_constant();
    append_cnot( circ, node_to_line[op1.node], target );
    append_cnot( circ, node_to_line[op2.node], target );
    return target;
  }

  void add_xor_uncompute( const xmg_function& op1, const xmg_function& op2, xmg_node node )
  {
    const auto target = node_to_line[node];
    append_cnot( circ, node_to_line[op1.node], target );
    append_cnot( circ, node_to_line[op2.node], target );

    line_to_node[target] = 0;
    is_garbage.reset( target );
    constants.push( target );
  }

  unsigned add_or( const xmg_function& op1, const xmg_function& op2 )
  {
    assert( op1.node && op2.node );
    assert( xmg.get_ref( op1.node ) );
    assert( xmg.get_ref( op2.node ) );

    const auto target = request_constant();
    append_toffoli( circ )( make_var( node_to_line[op1.node], op1.complemented ), make_var( node_to_line[op2.node], op2.complemented ) )( target );
    append_not( circ, target );
    return target;
  }

  void add_or_uncompute( const xmg_function& op1, const xmg_function& op2, xmg_node node )
  {
    const auto target = node_to_line[node];
    append_toffoli( circ )( make_var( node_to_line[op1.node], op1.complemented ), make_var( node_to_line[op2.node], op2.complemented ) )( target );
    append_not( circ, target );

    line_to_node[target] = 0;
    is_garbage.reset( target );
    constants.push( target );
  }

  unsigned add_and( const xmg_function& op1, const xmg_function& op2 )
  {
    assert( op1.node && op2.node );
    assert( xmg.get_ref( op1.node ) );
    assert( xmg.get_ref( op2.node ) );

    const auto target = request_constant();
    append_toffoli( circ )( make_var( node_to_line[op1.node], !op1.complemented ), make_var( node_to_line[op2.node], !op2.complemented ) )( target );
    return target;
  }

  void add_and_uncompute( const xmg_function& op1, const xmg_function& op2, xmg_node node )
  {
    const auto target = node_to_line[node];
    append_toffoli( circ )( make_var( node_to_line[op1.node], !op1.complemented ), make_var( node_to_line[op2.node], !op2.complemented ) )( target );

    line_to_node[target] = 0;
    is_garbage.reset( target );
    constants.push( target );
  }

  unsigned add_maj_inplace( const xmg_function& op1, const xmg_function& op2, const xmg_function& op3 )
  {
    assert( op1.node && op2.node && op3.node );
    assert( xmg.get_ref( op1.node ) );
    assert( xmg.get_ref( op2.node ) );
    assert( xmg.get_ref( op3.node ) );

    const auto l0 = node_to_line[op1.node];
    const auto l1 = node_to_line[op2.node];
    const auto l2 = node_to_line[op3.node];

    if ( op3.complemented )
    {
      append_not( circ, l2 );
    }

    append_cnot( circ, make_var( l0, !op1.complemented ), l1 );
    append_cnot( circ, make_var( l2, true ), l0 );
    append_toffoli( circ )( make_var( l0, !op1.complemented ), make_var( l1, op2.complemented ) )( l2 );

    line_to_node[l0] = 0;
    line_to_node[l1] = 0;
    is_garbage.set( l0 );
    is_garbage.set( l1 );

    return l2;
  }

  unsigned add_maj( const xmg_function& op1, const xmg_function& op2, const xmg_function& op3 )
  {
    assert( op1.node && op2.node && op3.node );
    assert( xmg.get_ref( op1.node ) );
    assert( xmg.get_ref( op2.node ) );
    assert( xmg.get_ref( op3.node ) );

    const auto target = request_constant();

    const auto l0 = node_to_line[op1.node];
    const auto l1 = node_to_line[op2.node];
    const auto l2 = node_to_line[op3.node];

    append_cnot( circ, make_var( l0, !op1.complemented ), l1 );
    append_cnot( circ, make_var( l2, !op3.complemented ), l0 );
    append_cnot( circ, make_var( l2, !op3.complemented ), target );
    append_toffoli( circ )( make_var( l0, !op1.complemented ), make_var( l1, op2.complemented ) )( target );
    append_cnot( circ, make_var( l2, !op3.complemented ), l0 );
    append_cnot( circ, make_var( l0, !op1.complemented ), l1 );

    return target;
  }

  void add_maj_uncompute( const xmg_function& op1, const xmg_function& op2, const xmg_function& op3, xmg_node node )
  {
    const auto target = node_to_line[node];

    const auto l0 = node_to_line[op1.node];
    const auto l1 = node_to_line[op2.node];
    const auto l2 = node_to_line[op3.node];

    append_cnot( circ, make_var( l0, !op1.complemented ), l1 );
    append_cnot( circ, make_var( l2, !op3.complemented ), l0 );
    append_toffoli( circ )( make_var( l0, !op1.complemented ), make_var( l1, op2.complemented ) )( target );
    append_cnot( circ, make_var( l2, !op3.complemented ), target );
    append_cnot( circ, make_var( l2, !op3.complemented ), l0 );
    append_cnot( circ, make_var( l0, !op1.complemented ), l1 );

    line_to_node[target] = 0;
    is_garbage.reset( target );
    constants.push( target );
  }

  unsigned add_computed_network( xmg_node node )
  {
    const auto target = request_constant();
    const auto& children = esmgr.get_leafs( node );

    std::vector<unsigned> line_map;

    for ( auto child : children )
    {
      if ( !child ) { continue; }
      assert( xmg.get_ref( child ) );

      line_map.push_back( node_to_line[child] );
    }
    line_map.push_back( target );

    L( boost::format( "[i] line map = %s" ) % any_join( line_map, " " ) );

    L( boost::format( "[i] add computed network with %d gates" ) % esmgr.get_network( node ).num_gates() );

    for ( const auto& g : esmgr.get_network( node ) )
    {
      assert( is_toffoli( g ) );

      gate::control_container controls;
      for ( const auto& c : g.controls() )
      {
        controls.push_back( make_var( line_map[c.line()], c.polarity() ) );
      }
      append_toffoli( circ, controls, line_map[g.targets().front()] );
    }

    return target;
  }

  void add_computed_network_uncompute( xmg_node node )
  {
    const auto target = node_to_line[node];
    const auto& children = esmgr.get_leafs( node );

    std::vector<unsigned> line_map;

    for ( auto child : children )
    {
      if ( !child ) { continue; }

      line_map.push_back( node_to_line[child] );
    }
    line_map.push_back( target );

    const auto pos = circ.num_gates();
    for ( const auto& g : esmgr.get_network( node ) )
    {
      assert( is_toffoli( g ) );

      gate::control_container controls;
      for ( const auto& c : g.controls() )
      {
        controls.push_back( make_var( line_map[c.line()], c.polarity() ) );
      }
      insert_toffoli( circ, pos, controls, line_map[g.targets().front()] );
    }

    line_to_node[target] = 0;
    is_garbage.reset( target );
    constants.push( target );
  }

private:
  circuit&                circ;
  xmg_graph&              xmg;
  std::vector<unsigned>   node_to_line;    /* maps nodes to lines */
  std::vector<unsigned>   line_to_node;    /* maps lines to nodes (0: constant or garbage) */
  boost::dynamic_bitset<> is_garbage;      /* is garbage line? */
  std::vector<int>        unccounter;      /* uncompute reference counter when not inplace */
  boost::dynamic_bitset<> is_output;       /* is output node? */

  std::stack<unsigned>    constants;

  /* settings */
  bool        inplace       = false;
  unsigned    var_threshold = 0u;
  std::string garbage_name  = "--";
  bool        verbose       = false;

  /* helper managers */
  dxs_exact_manager       esmgr;
};

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

bool direct_xmg_synthesis( circuit& circ, xmg_graph& xmg, const properties::ptr& settings, const properties::ptr& statistics )
{
  const auto bennett = get( settings, "bennett", false ); /* use benett + inplace */

  settings->set( "inplace", bennett );

  properties_timer t( statistics );

  direct_xmg_synthesis_manager mgr( circ, xmg, settings );
  const auto result = mgr.run();

  if ( bennett )
  {
    /* for later */
    circuit rev_copy;
    reverse_circuit( circ, rev_copy );

    auto garbage = circ.garbage();
    const auto outputs = circ.outputs();
    auto inputs = circ.inputs();

    for ( auto i = 0u; i < outputs.size(); ++i )
    {
      if ( garbage[i] ) { continue; }

      const auto line = add_line_to_circuit( circ, "0", outputs[i], false, false );
      append_cnot( circ, make_var( i, true ), line );

      garbage[i] = true;
      garbage.push_back( false );
      inputs.push_back( outputs[i] );
    }

    circ.set_garbage( garbage );
    circ.set_outputs( inputs );

    append_circuit( circ, rev_copy );
  }

  return result;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
