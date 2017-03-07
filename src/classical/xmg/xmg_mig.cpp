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

#include "xmg_mig.hpp"

#include <unordered_map>

#include <classical/mig/mig_utils.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

mig_function xmg_create_mig_top_down_rec( const xmg_graph& circ, const xmg_node& node, mig_graph& mig, std::unordered_map<xmg_node, mig_function>& node_to_function )
{
  const auto it = node_to_function.find( node );

  if ( it != node_to_function.end() )
  {
    return it->second;
  }

  mig_function f;
  const auto children = circ.children( node );

  if ( circ.is_maj( node ) )
  {
    f = mig_create_maj( mig,
                        xmg_create_mig_top_down_rec( circ, children[0u].node, mig, node_to_function ) ^ children[0u].complemented,
                        xmg_create_mig_top_down_rec( circ, children[1u].node, mig, node_to_function ) ^ children[1u].complemented,
                        xmg_create_mig_top_down_rec( circ, children[2u].node, mig, node_to_function ) ^ children[2u].complemented );
  }
  else if ( circ.is_xor( node ) )
  {
    f = mig_create_xor( mig,
                        xmg_create_mig_top_down_rec( circ, children[0u].node, mig, node_to_function ) ^ children[0u].complemented,
                        xmg_create_mig_top_down_rec( circ, children[1u].node, mig, node_to_function ) ^ children[1u].complemented );
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

mig_graph xmg_create_mig_topological( const xmg_graph& circ )
{
  mig_graph mig;
  mig_initialize( mig );

  std::vector<mig_function> node_to_function( circ.size() );
  node_to_function[0] = mig_get_constant( mig, false );

  for ( const auto& i : circ.inputs() )
  {
    node_to_function[i.first] = mig_create_pi( mig, i.second );
  }

  foreach_topological( circ.graph(), [&]( const xmg_node& node ) {
      switch ( circ.fanin_count( node ) )
      {
      case 2:
        {
          const auto children = circ.children( node );
          node_to_function[node] = mig_create_xor( mig,
                                                   node_to_function[children[0u].node] ^ children[0u].complemented,
                                                   node_to_function[children[1u].node] ^ children[1u].complemented );
        }
        break;

      case 3:
        {
          const auto children = circ.children( node );
          node_to_function[node] = mig_create_maj( mig,
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
    mig_create_po( mig, node_to_function[o.first.node] ^ o.first.complemented, o.second );
  }

  return mig;
}

mig_graph xmg_create_mig_top_down( const xmg_graph& circ, const xmg_function& f )
{
  mig_graph mig;
  mig_initialize( mig );

  std::unordered_map<xmg_node, mig_function> node_to_function;

  node_to_function.insert( {0, mig_get_constant( mig, false )} );

  for ( const auto& i : circ.inputs() )
  {
    node_to_function.insert( {i.first, mig_create_pi( mig, i.second )} );
  }

  const auto mig_f = xmg_create_mig_top_down_rec( circ, f.node, mig, node_to_function ) ^ f.complemented;

  mig_create_po( mig, mig_f, "f" );

  return mig;
}

xmg_graph xmg_from_mig( const mig_graph& mig )
{
  const auto& info = mig_info( mig );

  xmg_graph xmg( info.model_name );

  std::unordered_map<mig_node, xmg_function> node_to_function;
  node_to_function.insert( {0, xmg.get_constant( false )} );

  for ( const auto& input : info.inputs )
  {
    node_to_function.insert( {input, xmg.create_pi( info.node_names.at( input ) )} );
  }

  std::vector<mig_node> top( num_vertices( mig ) );
  boost::topological_sort( mig, top.begin() );

  for ( auto node : top )
  {
    if ( out_degree( node, mig ) == 0u ) continue;

    auto children = get_children( mig, node );
    node_to_function[node] = xmg.create_maj( node_to_function[children[0u].node] ^ children[0u].complemented,
                                             node_to_function[children[1u].node] ^ children[1u].complemented,
                                             node_to_function[children[2u].node] ^ children[2u].complemented );
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
