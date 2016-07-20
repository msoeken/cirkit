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

#include "xmg_aig.hpp"

#include <unordered_map>

#include <classical/utils/aig_utils.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

aig_function create_aig_top_down_rec( const xmg_graph& circ, const xmg_node& node, aig_graph& aig, std::unordered_map<xmg_node, aig_function>& node_to_function )
{
  const auto it = node_to_function.find( node );

  if ( it != node_to_function.end() )
  {
    return it->second;
  }

  aig_function f;
  const auto children = circ.children( node );

  if ( circ.is_maj( node ) )
  {
    f = aig_create_maj( aig,
                        create_aig_top_down_rec( circ, children[0u].node, aig, node_to_function ) ^ children[0u].complemented,
                        create_aig_top_down_rec( circ, children[1u].node, aig, node_to_function ) ^ children[1u].complemented,
                        create_aig_top_down_rec( circ, children[2u].node, aig, node_to_function ) ^ children[2u].complemented );
  }
  else if ( circ.is_xor( node ) )
  {
    f = aig_create_xor( aig,
                        create_aig_top_down_rec( circ, children[0u].node, aig, node_to_function ) ^ children[0u].complemented,
                        create_aig_top_down_rec( circ, children[1u].node, aig, node_to_function ) ^ children[1u].complemented );
  }
  else
  {
    assert( false );
  }

  node_to_function.insert( {node, f} );

  return f;
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

aig_graph create_aig_topological( const xmg_graph& circ )
{
  aig_graph aig;
  aig_initialize( aig );

  std::vector<aig_function> node_to_function( circ.size() );
  node_to_function[0] = aig_get_constant( aig, false );

  for ( const auto& i : circ.inputs() )
  {
    node_to_function[i.first] = aig_create_pi( aig, i.second );
  }

  foreach_topological( circ.graph(), [&]( const xmg_node& node ) {
      switch ( circ.fanin_count( node ) )
      {
      case 2:
        {
          const auto children = circ.children( node );
          node_to_function[node] = aig_create_xor( aig,
                                                   node_to_function[children[0u].node] ^ children[0u].complemented,
                                                   node_to_function[children[1u].node] ^ children[1u].complemented );
        }
        break;

      case 3:
        {
          const auto children = circ.children( node );
          node_to_function[node] = aig_create_maj( aig,
                                                   node_to_function[children[0u].node] ^ children[0u].complemented,
                                                   node_to_function[children[1u].node] ^ children[1u].complemented,
                                                   node_to_function[children[2u].node] ^ children[2u].complemented );
        }
        break;
      }

      return true;
    } );

  for ( const auto& o : circ.outputs() )
  {
    aig_create_po( aig, node_to_function[o.first.node] ^ o.first.complemented, o.second );
  }

  return aig;
}

aig_graph create_aig_top_down( const xmg_graph& circ, const xmg_function& f )
{
  aig_graph aig;
  aig_initialize( aig );

  std::unordered_map<xmg_node, aig_function> node_to_function;

  node_to_function.insert( {0, aig_get_constant( aig, false )} );

  for ( const auto& i : circ.inputs() )
  {
    node_to_function.insert( {i.first, aig_create_pi( aig, i.second )} );
  }

  const auto aig_f = create_aig_top_down_rec( circ, f.node, aig, node_to_function ) ^ f.complemented;

  aig_create_po( aig, aig_f, "f" );

  return aig;
}

xmg_graph xmg_from_aig( const aig_graph& aig )
{
  const auto& info = aig_info( aig );

  xmg_graph xmg( info.model_name );

  std::unordered_map<aig_node, xmg_function> node_to_function;
  node_to_function.insert( {0, xmg.get_constant( false )} );

  for ( const auto& input : info.inputs )
  {
    node_to_function.insert( {input, xmg.create_pi( info.node_names.at( input ) )} );
  }

  std::vector<aig_node> top( num_vertices( aig ) );
  boost::topological_sort( aig, top.begin() );

  for ( auto node : top )
  {
    if ( out_degree( node, aig ) == 0u ) continue;

    auto children = get_children( aig, node );
    node_to_function[node] = xmg.create_and( node_to_function[children[0u].node] ^ children[0u].complemented,
                                             node_to_function[children[1u].node] ^ children[1u].complemented );
  }

  for ( const auto& o : info.outputs )
  {
    xmg.create_po( node_to_function[o.first.node] ^ o.first.complemented, o.second );
  }

  return xmg;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
