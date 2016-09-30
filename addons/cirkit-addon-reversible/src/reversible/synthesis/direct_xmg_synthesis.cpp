/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2016  EPFL
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

#include <queue>
#include <stack>
#include <vector>

#include <boost/dynamic_bitset.hpp>

#include <core/utils/range_utils.hpp>
#include <core/utils/timer.hpp>
#include <reversible/functions/add_circuit.hpp>
#include <reversible/functions/add_gates.hpp>
#include <reversible/functions/add_line_to_circuit.hpp>
#include <reversible/functions/reverse_circuit.hpp>

#define L(x) if ( verbose ) { std::cout << x << std::endl; }

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

class direct_xmg_synthesis_manager
{
public:
  direct_xmg_synthesis_manager( circuit& circ, xmg_graph& xmg, const properties::ptr& settings )
    : circ( circ ),
      xmg( xmg ),
      node_to_line( xmg.size() ),
      line_to_node( xmg.inputs().size() ),
      is_garbage( xmg.inputs().size() )
  {
    inplace      = get( settings, "inplace",      inplace );
    garbage_name = get( settings, "garbage_name", garbage_name );
    verbose      = get( settings, "verbose",      verbose );

    /* initialize reference counting */
    xmg.init_refs();
    xmg.inc_output_refs();
  }

  bool run()
  {
    add_inputs();

    for ( auto node : xmg.topological_nodes() )
    {
      if ( xmg.is_input( node ) ) { continue; }

      const auto line = add_node( node );

      /* decrement reference counters and update line for node */
      node_to_line[node] = line;
      line_to_node[line] = node;
      L( boost::format( "[i] node %d -> line %d" ) % node % node_to_line[node] );

      if ( true )
      {
        for ( const auto& c : xmg.children( node ) )
        {
          xmg.dec_ref( c.node );
          if ( xmg.get_ref( c.node ) == 0 && !xmg.is_input( c.node ) )
          {
            if ( can_uncompute( c.node ) )
            {
              L( boost::format( "[i] uncompute node %d" ) % c.node );
              add_node_uncompute( c.node );
            }
            else
            {
              L( boost::format( "[i] cannot uncompute node %d" ) % c.node );
              is_garbage[node_to_line[c.node]] = true;
            }
          }
        }
      }
    }

    add_outputs();

    return true;
  }

private:
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
    for ( const auto& c : xmg.children( node ) )
    {
      if ( c.node == 0 ) { continue; } /* no constants */

      if ( line_to_node[node_to_line[c.node]] != c.node )
      {
        return false;
      }
    }

    return true;
  }

  unsigned add_node( xmg_node node )
  {
    const auto children = xmg.children( node );

    if ( xmg.is_xor( node ) ) /* XOR */
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

    if ( xmg.is_xor( node ) ) /* XOR */
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
        assert( xmg.is_input( line_to_node[i] ) );
        garbage[i] = true; /* but not really */
        outputs[i] = xmg.input_name( line_to_node[i] );
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

private:
  circuit&                circ;
  xmg_graph&              xmg;
  std::vector<unsigned>   node_to_line;    /* maps nodes to lines */
  std::vector<unsigned>   line_to_node;    /* maps lines to nodes (0: constant or garbage) */
  boost::dynamic_bitset<> is_garbage;      /* is garbage line? */

  std::stack<unsigned>    constants;

  /* settings */
  bool        inplace      = false;
  std::string garbage_name = "--";
  bool        verbose      = false;
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
