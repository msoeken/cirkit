/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2016  EPFL
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

#include "direct_xmg_synthesis.hpp"

#include <vector>

#include <core/utils/timer.hpp>
#include <reversible/functions/add_gates.hpp>
#include <reversible/functions/add_line_to_circuit.hpp>

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
      node_to_line( xmg.size() )
  {
    garbage_name = get( settings, "garbage_name", garbage_name );

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

      const auto children = xmg.children( node );
      unsigned line;

      if ( xmg.is_xor( node ) ) /* XOR */
      {
        if ( xmg.get_ref( children[0u].node ) == 1u )
        {
          /* set a <- a XOR b */
          line = add_xor_inplace( children[0u], children[1u] );
        }
        else if ( xmg.get_ref( children[1u].node ) == 1u )
        {
          /* set b <- a XOR b */
          line = add_xor_inplace( children[1u], children[0u] );
        }
        else
        {
          line = add_xor( children[0u], children[1u] );
        }
      }
      else /* MAJ, AND, OR */
      {
        if ( children[0u].node == 0u )
        {
          if ( children[0u].complemented ) /* OR */
          {
            line = add_or( children[1u], children[2u] );
          }
          else /* AND */
          {
            line = add_and( children[1u], children[2u] );
          }
        }
        else /* MAJ */
        {
          if ( xmg.get_ref( children[0u].node ) == 1u && xmg.get_ref( children[1u].node ) == 1u && xmg.get_ref( children[2u].node ) == 1u )
          {
            line = add_maj_inplace( children[0u], children[1u], children[2u] );
          }
          else
          {
            line = add_maj( children[0u], children[1u], children[2u] );
          }
        }
      }

      /* decrement reference counters and update line for node */
      for ( const auto& c : children )
      {
        xmg.dec_ref( c.node );
      }
      node_to_line[node] = line;
    }

    add_outputs();

    return true;
  }

private:
  void add_inputs()
  {
    for ( const auto& input : xmg.inputs() )
    {
      node_to_line[input.first] = add_line_to_circuit( circ, input.second, garbage_name, constant(), true );
    }
  }

  void add_outputs()
  {
    auto garbage = circ.garbage();
    auto outputs = circ.outputs();

    for ( const auto& output : xmg.outputs() )
    {
      const auto l = node_to_line[output.first.node];
      assert( garbage[l] == true );

      garbage[l] = false;
      outputs[l] = output.second;

      if ( output.first.complemented )
      {
        append_not( circ, l );
      }
    }

    circ.set_garbage( garbage );
    circ.set_outputs( outputs );
  }

  unsigned add_xor_inplace( const xmg_function& dest, const xmg_function& src )
  {
    append_cnot( circ, node_to_line[src.node], node_to_line[dest.node] );
    return node_to_line[dest.node];
  }

  unsigned add_xor( const xmg_function& op1, const xmg_function& op2 )
  {
    const auto target = add_line_to_circuit( circ, "0", garbage_name, false, true );
    append_cnot( circ, node_to_line[op1.node], target );
    append_cnot( circ, node_to_line[op2.node], target );
    return target;
  }

  unsigned add_or( const xmg_function& op1, const xmg_function& op2 )
  {
    const auto target = add_line_to_circuit( circ, "0", garbage_name, false, true );
    append_toffoli( circ )( make_var( node_to_line[op1.node], op1.complemented ), make_var( node_to_line[op2.node], op2.complemented ) )( target );
    append_not( circ, target );
    return target;
  }

  unsigned add_and( const xmg_function& op1, const xmg_function& op2 )
  {
    const auto target = add_line_to_circuit( circ, "0", garbage_name, false, true );
    append_toffoli( circ )( make_var( node_to_line[op1.node], !op1.complemented ), make_var( node_to_line[op2.node], !op2.complemented ) )( target );
    return target;
  }

  unsigned add_maj_inplace( const xmg_function& op1, const xmg_function& op2, const xmg_function& op3 )
  {
    append_cnot( circ, make_var( node_to_line[op1.node], !op1.complemented ), node_to_line[op2.node] );
    append_cnot( circ, make_var( node_to_line[op3.node], !op3.complemented ), node_to_line[op1.node] );
    append_toffoli( circ )( make_var( node_to_line[op1.node], !op1.complemented ), make_var( node_to_line[op2.node], op2.complemented ) )( node_to_line[op3.node] );
    return node_to_line[op3.node];
  }

  unsigned add_maj( const xmg_function& op1, const xmg_function& op2, const xmg_function& op3 )
  {
    const auto target = add_line_to_circuit( circ, "0", garbage_name, false, true );

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

private:
  circuit&              circ;
  xmg_graph&            xmg;
  std::vector<unsigned> node_to_line;

  /* settings */
  std::string garbage_name = "--";
};

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

bool direct_xmg_synthesis( circuit& circ, xmg_graph& xmg, const properties::ptr& settings, const properties::ptr& statistics )
{
  const auto garbage_name = get( settings, "garbage_name", std::string( "--" ) );

  properties_timer t( statistics );

  direct_xmg_synthesis_manager mgr( circ, xmg, settings );
  return mgr.run();
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
