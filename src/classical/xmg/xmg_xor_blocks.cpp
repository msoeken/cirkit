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

#include "xmg_xor_blocks.hpp"

#include <stack>
#include <unordered_map>

#include <boost/dynamic_bitset.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/range/algorithm.hpp>

#include <core/utils/timer.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

std::vector<xmg_node> get_start_nodes( xmg_graph& xmg )
{
  std::vector<xmg_node> top( xmg.size() );
  boost::topological_sort( xmg.graph(), top.begin() );

  std::vector<xmg_node> start_nodes;
  std::unordered_map<xmg_node, unsigned> po_to_top_id;

  for ( const auto& output : xmg.outputs() )
  {
    po_to_top_id.insert( {output.first.node, std::distance( top.begin(), boost::find( top, output.first.node ) )} );
    start_nodes.push_back( output.first.node );
  }

  boost::sort( start_nodes, [&po_to_top_id]( xmg_node v1, xmg_node v2 ) { return po_to_top_id[v1] < po_to_top_id[v2]; } );

  return start_nodes;
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

std::unordered_map<xmg_node, xmg_xor_block_t> xmg_find_xor_blocks( xmg_graph& xmg,
                                                                   const properties::ptr& settings,
                                                                   const properties::ptr& statistics )
{
  /* timing */
  properties_timer t( statistics );

  xmg.compute_fanout();

  /* prepare stack */
  std::stack<xmg_node> stack;
  boost::for_each( get_start_nodes( xmg ), [&stack]( xmg_node v ) { stack.push( v ); } );

  boost::dynamic_bitset<> processed( xmg.size() );

  std::unordered_map<xmg_node, xmg_xor_block_t> result;
  std::vector<xmg_node> internals; /* internal XOR gates in blocks */

  while ( !stack.empty() )
  {
    auto n = stack.top();
    stack.pop();

    /* already seen? */
    if ( processed[n] ) { continue; }

    /* input? */
    if ( xmg.is_input( n ) ) { processed[n] = true; continue; }

    /* MAJ? */
    if ( xmg.is_maj( n ) )
    {
      for ( auto c : xmg.children( n ) )
      {
        stack.push( c.node );
      }
      processed[n] = true;
      continue;
    }

    /* XOR! */
    assert( xmg.is_xor( n ) );

    std::stack<xmg_node> block_stack;
    block_stack.push( n );
    std::vector<xmg_node> block;
    auto block_size = 0u;

    while ( !block_stack.empty() )
    {
      auto bn = block_stack.top();
      block_stack.pop();

      if ( !xmg.is_xor( bn ) || ( xmg.fanout_count( bn ) > 1u && bn != n ) )
      {
        /* found a child */
        if ( boost::find( block, bn ) == block.end() )
        {
          block.push_back( bn );
        }
        continue;
      }

      ++block_size;
      processed[bn] = true;
      for ( const auto& c : xmg.children( bn ) )
      {
        assert( !c.complemented );
        block_stack.push( c.node );
      }
      if ( bn != n && boost::find( internals, bn ) == internals.end() )
      {
        internals.push_back( bn );
      }
    }

    //if ( block_size > 1u )
    //{
    result.insert( {n, block} );
    //}

    for ( auto bn : block )
    {
      stack.push( bn );
    }
  }

  set( statistics, "internals", internals );

  return result;
}


}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
