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

#include "xmg_lut.hpp"

#include <iostream>

#include <boost/range/iterator_range.hpp>

#include <core/utils/conversion_utils.hpp>
#include <core/utils/graph_utils.hpp>
#include <classical/utils/truth_table_utils.hpp>
#include <classical/xmg/xmg_cover.hpp>
#include <classical/xmg/xmg_utils.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

void xmg_to_lut_add_cut( const xmg_graph& xmg, lut_graph_t& lut, xmg_node n,
                         std::vector<lut_vertex_t>& node_to_node, boost::dynamic_bitset<>& visited )
{
  if ( visited[n] ) { return; }

  assert( xmg.cover().has_cut( n ) );

  /* two pass */
  std::vector<xmg_node> leafs;
  for ( auto l : xmg.cover().cut( n ) )
  {
    xmg_to_lut_add_cut( xmg, lut, l, node_to_node, visited );
    leafs.push_back( l );
  }

  auto tt = xmg_simulate_cut( xmg, n, leafs );

  auto node = add_vertex( lut );
  boost::get( boost::vertex_lut_type, lut )[node] = lut_type_t::internal;
  boost::get( boost::vertex_lut, lut )[node ] = tt_to_hex( tt );
  node_to_node[n] = node;
  visited.set( n );

  for ( auto l : leafs )
  {
    add_edge( node, node_to_node[l], lut );
  }
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

lut_graph_t xmg_to_lut_graph( const xmg_graph& xmg )
{
  /* xmg needs to have a cover */
  if ( !xmg.has_cover() )
  {
    std::cout << "[e] xmg has no cover" << std::endl;
    assert( false );
  }

  /* graph */
  lut_graph_t lut;
  const auto& name = get( boost::vertex_name, lut );
  const auto& gate = get( boost::vertex_lut_type, lut );
  const auto& luts = get( boost::vertex_lut, lut );

  boost::dynamic_bitset<> visited( xmg.size() );
  std::vector<lut_vertex_t> node_to_node( xmg.size() );

  /* constants */
  auto gnd = add_vertex( lut );
  name[gnd] = "gnd";
  gate[gnd] = lut_type_t::gnd;
  visited.set( 0 );
  node_to_node[0] = gnd;

  auto vdd = add_vertex( lut );
  name[vdd] = "vdd";
  gate[vdd] = lut_type_t::vdd;

  /* copy inputs */
  for ( const auto& pi : xmg.inputs() )
  {
    auto n = add_vertex( lut );
    name[n] = pi.second;
    gate[n] = lut_type_t::pi;

    visited.set( pi.first );
    node_to_node[pi.first] = n;
  }

  std::map<unsigned, lut_vertex_t> output_to_node;
  for ( const auto& po : xmg.outputs() )
  {
    xmg_to_lut_add_cut( xmg, lut, po.first.node, node_to_node, visited );

    /* even for non-complemented nodes, and odd for complemented */
    output_to_node[po.first.node << 1u] = node_to_node[po.first.node];
  }

  /* make POs */
  auto inedges = precompute_ingoing_edges( xmg.graph() );
  visited.reset();
  for ( const auto& po : xmg.outputs() )
  {
    lut_vertex_t target;
    const auto node = po.first.node;

    // std::cout << "[i] process output " << po.second << " (node: " << node << ", complemented: " << po.first.complemented << ")" << std::endl;

    /* output points to input */
    if ( xmg.is_input( node ) )
    {
      if ( po.first.complemented )
      {
        if ( node == 0u )
        {
          target = vdd;
        }
        else
        {
          auto it = output_to_node.find( ( node << 1u ) + 1u );
          if ( it == output_to_node.end() )
          {
            target = add_vertex( lut );
            add_edge( target, node_to_node[node], lut );
            gate[target] = lut_type_t::internal;
            luts[target] = "1";
            output_to_node[( node << 1u ) + 1u] = target;
          }
          else
          {
            target = it->second;
          }
        }
      }
      else
      {
        target = node_to_node[node];
      }
    }
    /* output is complemented */
    else if ( po.first.complemented )
    {
      /* already computed? */
      auto it = output_to_node.find( ( node << 1u ) + 1u );
      if ( it != output_to_node.end() )
      {
        target = it->second;
      }
      /* does it have outgoing edges or there is a positive version */
      else if ( inedges.find( node ) != inedges.end() || visited[node] )
      {
        // std::cout << "[i]  - case: more fanout or other postive" << std::endl;
        target = add_vertex( lut );

        for ( auto child : boost::make_iterator_range( boost::adjacent_vertices( node_to_node[node], lut ) ) )
        {
          add_edge( target, child, lut );
        }

        gate[target] = lut_type_t::internal;
        const auto& ltt = luts[node_to_node[node]];
        luts[target] = invert_hex( ltt );
        output_to_node[ ( node << 1u ) + 1u ] = target;
      }
      else
      {
        // std::cout << "[i]  - case: invert and overwrite positive node" << std::endl;
        target = output_to_node[node << 1u];
        const auto& ltt = luts[target];
        luts[target] = invert_hex( ltt );
        //luts[target] = std::make_pair( ltt.first, ( ~boost::dynamic_bitset<>( 1u << ltt.first, ltt.second ) ).to_ulong() );
        output_to_node[ ( node << 1u ) + 1u ] = target;
        output_to_node.erase( node << 1u );
      }
    }
    else
    {
      auto it = output_to_node.find( node << 1u );
      visited.set( node );
      if ( it == output_to_node.end() )
      {
        /* recreate positive version */
        target = add_vertex( lut );

        for ( auto child : boost::make_iterator_range( boost::adjacent_vertices( output_to_node[( node << 1u ) + 1u], lut ) ) )
        {
          add_edge( target, child, lut );
        }

        const auto& ltt = luts[output_to_node[( node << 1u ) + 1u]];
        //luts[target] = std::make_pair( ltt.first, ( ~boost::dynamic_bitset<>( 1u << ltt.first, ltt.second ) ).to_ulong() );
        luts[target] = invert_hex( ltt );
        output_to_node[node << 1u] = target;
      }
      else
      {
        target = it->second;
      }
    }

    auto n = add_vertex( lut );
    name[n] = po.second;
    gate[n] = lut_type_t::po;
    add_edge( n, target, lut );
  }

  return lut;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
