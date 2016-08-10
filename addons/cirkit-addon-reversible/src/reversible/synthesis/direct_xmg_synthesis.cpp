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

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

bool direct_xmg_synthesis( circuit& circ, const xmg_graph& xmg, const properties::ptr& settings, const properties::ptr& statistics )
{
  const auto garbage_name = get( settings, "garbage_name", std::string( "--" ) );

  properties_timer t( statistics );

  std::vector<unsigned> node_to_line( xmg.size() );

  for ( const auto& input : xmg.inputs() )
  {
    node_to_line[input.first] = add_line_to_circuit( circ, input.second, garbage_name, constant(), true );
  }

  for ( const auto& node : xmg.topological_nodes() )
  {
    if ( xmg.is_input( node ) ) { continue; }

    auto target = node_to_line[node] = add_line_to_circuit( circ, "0", garbage_name, false, true );
    const auto children = xmg.children( node );

    if ( xmg.is_xor( node ) ) /* XOR */
    {
      const auto l0 = node_to_line[children[0u].node];
      const auto l1 = node_to_line[children[1u].node];

      append_cnot( circ, make_var( l0, !children[0u].complemented ), target );
      append_cnot( circ, make_var( l1, !children[1u].complemented ), target );
    }
    else /* OR, AND, MAJ */
    {
      const auto l1 = node_to_line[children[1u].node];
      const auto l2 = node_to_line[children[2u].node];

      if ( children[0u].node == 0u )
      {
        if ( children[0u].complemented ) /* OR */
        {
          append_toffoli( circ )( make_var( l1, children[1u].complemented ), make_var( l2, children[2u].complemented ) )( target );
          append_not( circ, target );
        }
        else /* AND */
        {
          append_toffoli( circ )( make_var( l1, !children[1u].complemented ), make_var( l2, !children[2u].complemented ) )( target );
        }
      }
      else /* MAJ */
      {
        const auto l0 = node_to_line[children[0u].node];

        append_cnot( circ, make_var( l0, !children[0u].complemented ), l1 );
        append_cnot( circ, make_var( l2, !children[2u].complemented ), l0 );
        append_cnot( circ, make_var( l2, !children[2u].complemented ), target );
        append_toffoli( circ )( make_var( l0, !children[0u].complemented ), make_var( l1, children[1u].complemented ) )( target );
        append_cnot( circ, make_var( l2, !children[2u].complemented ), l0 );
        append_cnot( circ, make_var( l0, !children[0u].complemented ), l1 );
      }
    }
  }

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

  return true;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
