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

#include "xmg_utils.hpp"

#include <boost/format.hpp>
#include <boost/range/iterator_range.hpp>
#include <range/v3/action/remove_if.hpp>

#include <classical/xmg/xmg_cover.hpp>
#include <classical/xmg/xmg_simulate.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

int compute_depth_rec( const xmg_graph& xmg, xmg_node node, std::vector<int>& depths )
{
  if ( depths[node] >= 0 ) return depths[node];

  if ( xmg.is_input( node ) )
  {
    return depths[node] = 0;
  }
  else
  {
    int depth = 0;
    for ( const auto& c : xmg.children( node ) )
    {
      depth = std::max( compute_depth_rec( xmg, c.node, depths ), depth );
    }
    return depths[node] = ( depth + 1 );
  }
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

unsigned compute_depth( const xmg_graph& xmg )
{
  std::vector<int> depths( xmg.size(), -1 );

  int depth = -1;

  for ( const auto& output : xmg.outputs() )
  {
    depth = std::max( compute_depth_rec( xmg, output.first.node, depths ), depth );
  }

  return static_cast<unsigned>( depth );
}

void xmg_print_stats( const xmg_graph& xmg, std::ostream& os )
{
  auto name = xmg.name();
  if ( name.empty() )
  {
    name = "(unnamed)";
  }

  os << boost::format( "[i] %20s: i/o = %7d / %7d  maj = %7d  xor = %7d  lev = %4d" ) % name % xmg.inputs().size() % xmg.outputs().size() % xmg.num_maj() % xmg.num_xor() % compute_depth( xmg );

  if ( xmg.has_cover() )
  {
    os << boost::format( "  luts (k: %d) = %7d" ) % xmg.cover().cut_size() % xmg.cover().lut_count();
  }

  os << std::endl;
}

unsigned compute_pure_maj_count( const xmg_graph& xmg )
{
  auto total = 0u;

  for ( auto v : xmg.nodes() )
  {
    if ( xmg.is_pure_maj( v ) )
    {
      ++total;
    }
  }

  return total;
}

std::vector<unsigned> xmg_compute_levels( const xmg_graph& xmg )
{
  std::vector<unsigned> levels( xmg.size() );
  for ( const auto& n : xmg.topological_nodes() )
  {
    if ( xmg.is_input( n ) )
    {
      levels[n] = 0u;
    }
    else
    {
      auto level = 0u;
      for ( const auto& c : xmg.children( n ) )
      {
        level = std::max( level, levels[c.node] );
      }
      levels[n] = level + 1u;
    }
  }
  return levels;
}

std::vector< std::vector<xmg_node> > xmg_levelize( xmg_graph& xmg )
{
  const auto depth = compute_depth( xmg );
  xmg.compute_levels();

  std::vector< std::vector<xmg_node> > levels( depth+1u );
  for ( const auto& n : xmg.nodes() )
  {
    assert(levels.size() > xmg.level(n));
    levels[xmg.level(n)] += n;
  }

  return levels;
}

std::stack<xmg_node> xmg_output_stack( const xmg_graph& xmg )
{
  std::stack<xmg_node> stack;

  for ( const auto& output : xmg.outputs() )
  {
    stack.push( output.first.node );
  }

  return stack;
}

std::deque<xmg_node> xmg_output_deque( const xmg_graph& xmg )
{
  std::deque<xmg_node> deque;

  for ( const auto& output : xmg.outputs() )
  {
    deque.push_back( output.first.node );
  }

  return deque;
}

tt xmg_simulate_cut( const xmg_graph& xmg, xmg_node root, const std::vector<xmg_node>& leafs )
{
  std::map<xmg_node, tt> inputs;
  auto i = 0u;
  for ( auto child : leafs )
  {
    inputs[child] = tt_nth_var( i++ );
  }

  xmg_tt_simulator tt_sim;
  xmg_partial_node_assignment_simulator<tt> sim( tt_sim, inputs, tt_const0() );

  auto tt = simulate_xmg_node( xmg, root, sim );

  if ( leafs.size() < 6u )
  {
    tt_shrink( tt, leafs.size() );
  }

  return tt;
}

boost::dynamic_bitset<> xmg_output_mask( const xmg_graph& xmg )
{
  boost::dynamic_bitset<> mask( xmg.size() );

  for ( const auto& o : xmg.outputs() )
  {
    mask.set( o.first.node );
  }

  return mask;
}

xmg_edge xmg_get_edge( const xmg_graph& xmg, xmg_node parent, xmg_node child )
{
  for ( const auto& edge : boost::make_iterator_range( boost::out_edges( parent, xmg.graph() ) ) )
  {
    if ( boost::target( edge, xmg.graph() ) == child )
    {
      return edge;
    }
  }

  throw boost::str( boost::format( "no edge between %d and %d" ) % parent % child );
}

void xmg_delete_po_by_node( xmg_graph& xmg, xmg_node node )
{
  ranges::action::remove_if( xmg.outputs(), [node]( const xmg_graph::output_vec_t::value_type& v ) { return v.first.node == node; } );
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
