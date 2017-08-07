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

#include "xmg.hpp"

#include <range/v3/iterator_range.hpp>

#include <classical/xmg/xmg_bitmarks.hpp>
#include <classical/xmg/xmg_cover.hpp>
#include <classical/xmg/xmg_utils.hpp>
#include <core/utils/range_utils.hpp>

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

/******************************************************************************
 * header                                                                     *
 ******************************************************************************/

xmg_graph::xmg_graph( const std::string& name )
  : constant( add_vertex( g ) ),
    _name( name ),
    _complement( boost::get( boost::edge_complement, g ) ),
    _bitmarks( std::make_shared<xmg_bitmarks>() )
{
  assert( constant == 0 );
}

void xmg_graph::compute_fanout()
{
  fanout.update( [this]() { return precompute_in_degrees( g ); } );
}

void xmg_graph::compute_parents()
{
  parentss.update( [this]() { return precompute_ingoing_vertices( g ); } );
}

void xmg_graph::compute_levels()
{
  levels.update( [this]() { return xmg_compute_levels( *this ); } );
}

xmg_function xmg_graph::get_constant( bool value ) const
{
  return xmg_function( constant, value );
}

xmg_function xmg_graph::create_pi( const std::string& name )
{
  const auto node = add_vertex( g );
  _input_to_id.insert( {node, _inputs.size()} );
  _inputs.push_back( {node, name} );
  return xmg_function( node );
}

void xmg_graph::create_po( const xmg_function& f, const std::string& name )
{
  _outputs.push_back( {f, name} );
}

void xmg_graph::delete_po( unsigned index )
{
  if ( index < _outputs.size() )
  {
    _outputs.erase( _outputs.begin() + index );
  }
}

xmg_function xmg_graph::create_maj( const xmg_function& a, const xmg_function& b, const xmg_function& c )
{
  /* Special cases */
  if ( a == b )  { return a; }
  if ( a == c )  { return a; }
  if ( b == c )  { return b; }
  if ( a == !b ) { return c; }
  if ( a == !c ) { return b; }
  if ( b == !c ) { return a; }

  /* structural hashing */
  xmg_function children[] = {a, b, c};
  std::sort( children, children + 3 );

  auto node_complement = false;
  if ( _enable_inverter_propagation &&
       static_cast<unsigned>( children[0].complemented ) + static_cast<unsigned>( children[1].complemented ) + static_cast<unsigned>( children[2].complemented ) >= 2u )
  {
    node_complement = true;
    children[0].complemented = !children[0].complemented;
    children[1].complemented = !children[1].complemented;
    children[2].complemented = !children[2].complemented;
  }

  auto key = std::make_tuple( children[0], children[1], children[2] );

  const auto it = maj_strash.find( key );
  if ( _enable_structural_hashing && it != maj_strash.end() )
  {
    return xmg_function( it->second, node_complement );
  }

  /* insert node */
  const auto node = add_vertex( g );
  ++_num_maj;

  const auto ea = add_edge( node, children[0].node, g ).first;
  const auto eb = add_edge( node, children[1].node, g ).first;
  const auto ec = add_edge( node, children[2].node, g ).first;

  _complement[ea] = children[0].complemented;
  _complement[eb] = children[1].complemented;
  _complement[ec] = children[2].complemented;

  mark_as_modified();

  maj_strash[key] = node;
  return xmg_function( node, node_complement );
}

xmg_function xmg_graph::create_xor( const xmg_function& a, const xmg_function& b )
{
  if ( _native_xor )
  {
    /* special cases */
    if ( a == b ) { return get_constant( false ); }
    if ( a == !b) { return get_constant( true ); }
    if ( a.node == constant ) { return xmg_function( b.node, b.complemented != a.complemented ); }
    if ( b.node == constant ) { return xmg_function( a.node, a.complemented != b.complemented ); }

    /* structural hashing */
    auto key = a.node < b.node ? std::make_pair( a, b ) : std::make_pair( b, a );

    /* normalize polarities */
    auto node_complement = false;

    if ( _enable_inverter_propagation )
    {
      node_complement = key.first.complemented != key.second.complemented;
      key.first.complemented = key.second.complemented = false;
    }

    const auto it = xor_strash.find( key );
    if ( _enable_structural_hashing && it != xor_strash.end() )
    {
      return xmg_function( it->second, node_complement );
    }

    /* insert node */
    const auto node = add_vertex( g );
    ++_num_xor;

    const auto ea = add_edge( node, key.first.node, g ).first;
    const auto eb = add_edge( node, key.second.node, g ).first;

    _complement[ea] = key.first.complemented;
    _complement[eb] = key.second.complemented;

    mark_as_modified();

    xor_strash[key] = node;
    return xmg_function( node, node_complement );
  }
  else
  {
    return create_maj( !a, create_or( a, b ), create_and( a, !b ) );
  }
}

xmg_function xmg_graph::create_and( const xmg_function& a, const xmg_function& b )
{
  return create_maj( get_constant( false ), a, b );
}

xmg_function xmg_graph::create_or( const xmg_function& a, const xmg_function& b )
{
  return create_maj( get_constant( true ), a, b );
}

xmg_function xmg_graph::create_ite( const xmg_function& c, const xmg_function& t, const xmg_function& e )
{
  return create_maj( create_xor( c, e ), t, e );
}

xmg_function xmg_graph::create_nary_and( const std::vector<xmg_function>& ops )
{
  return balanced_accumulate( ops.begin(), ops.end(), [this]( const xmg_function& a, const xmg_function& b ) { return create_and( a, b ); } );
}

xmg_function xmg_graph::create_nary_or( const std::vector<xmg_function>& ops )
{
  return balanced_accumulate( ops.begin(), ops.end(), [this]( const xmg_function& a, const xmg_function& b ) { return create_or( a, b ); } );
}

unsigned xmg_graph::fanin_count( node_t n ) const
{
  return boost::out_degree( n, g );
}

unsigned xmg_graph::fanout_count( node_t n ) const
{
  return (*fanout)[n];
}

const std::vector<xmg_graph::node_t>& xmg_graph::parents( node_t n ) const
{
  return (*parentss)[n];
}

unsigned xmg_graph::level( node_t n ) const
{
  return (*levels)[n];
}

bool xmg_graph::is_input( node_t n ) const
{
  return fanin_count( n ) == 0u;
}

bool xmg_graph::is_maj( node_t n ) const
{
  return fanin_count( n ) == 3u;
}

bool xmg_graph::is_pure_maj( node_t n ) const
{
  return fanin_count( n ) == 3u && ( *( boost::adjacent_vertices( n, g ).first ) != 0 );
}

bool xmg_graph::is_xor( node_t n ) const
{
  return fanin_count( n ) == 2u;
}

const std::string& xmg_graph::name() const
{
  return _name;
}

void xmg_graph::set_name( const std::string& name )
{
  _name = name;
}

std::size_t xmg_graph::size() const
{
  return num_vertices( g );
}

unsigned xmg_graph::num_gates() const
{
  return _num_maj + _num_xor;
}

unsigned xmg_graph::num_maj() const
{
  return _num_maj;
}

unsigned xmg_graph::num_xor() const
{
  return _num_xor;
}

const xmg_graph::graph_t& xmg_graph::graph() const
{
  return g;
}

xmg_graph::graph_t& xmg_graph::graph()
{
  return g;
}

const xmg_graph::input_vec_t& xmg_graph::inputs() const
{
  return _inputs;
}

const xmg_graph::output_vec_t& xmg_graph::outputs() const
{
  return _outputs;
}

xmg_graph::input_vec_t& xmg_graph::inputs()
{
  return _inputs;
}

xmg_graph::output_vec_t& xmg_graph::outputs()
{
  return _outputs;
}

const std::string& xmg_graph::input_name( xmg_node n ) const
{
  return _inputs[_input_to_id.at( n )].second;
}

const unsigned xmg_graph::input_index( xmg_node n ) const
{
  return _input_to_id.at( n );
}

xmg_graph::vertex_range_t xmg_graph::nodes() const
{
  const auto v = boost::vertices( g );
  return ranges::make_iterator_range( v.first, v.second );
}

xmg_graph::edge_range_t xmg_graph::edges() const
{
  const auto e = boost::edges( g );
  return ranges::make_iterator_range( e.first, e.second );
}

std::vector<xmg_function> xmg_graph::children( xmg_node n ) const
{
  std::vector<xmg_function> c;
  for ( const auto& e : boost::make_iterator_range( boost::out_edges( n, g ) ) )
  {
    c.push_back( xmg_function( boost::target( e, g ), _complement[e] ) );
  }
  return c;
}

std::vector<xmg_graph::node_t> xmg_graph::topological_nodes() const
{
  std::vector<node_t> top( num_vertices( g ) );
  boost::topological_sort( g, top.begin() );
  return top;
}

bool xmg_graph::has_cover() const
{
  return (bool)_cover;
}

const xmg_cover& xmg_graph::cover() const
{
  return *_cover;
}

void xmg_graph::set_cover( const xmg_cover& other )
{
  if ( !_cover )
  {
    _cover = std::make_shared<xmg_cover>( 0u, *this );
  }
  *_cover = other;
}

void xmg_graph::init_refs()
{
  compute_fanout();
  ref_count = *fanout;
}

unsigned xmg_graph::get_ref( xmg_node n ) const
{
  return ref_count[n];
}

unsigned xmg_graph::inc_ref( xmg_node n )
{
  return ref_count[n]++;
}

unsigned xmg_graph::dec_ref( xmg_node n )
{
  assert( ref_count[n] > 0 );
  return --ref_count[n];
}

void xmg_graph::inc_output_refs()
{
  for ( const auto& output : outputs() )
  {
    ++ref_count[output.first.node];
  }
}

xmg_bitmarks& xmg_graph::bitmarks()
{
  return *_bitmarks;
}

const xmg_bitmarks& xmg_graph::bitmarks() const
{
  return *_bitmarks;
}

void xmg_graph::mark_as_modified()
{
  fanout.make_dirty();
  parentss.make_dirty();
  levels.make_dirty();
}

/******************************************************************************
 * xmg_fuction                                                            *
 ******************************************************************************/

xmg_function::xmg_function( xmg_node node, bool complemented )
  : node( node ),
    complemented( complemented )
{
}

bool xmg_function::operator==( const xmg_function& other ) const
{
  return node == other.node && complemented == other.complemented;
}

bool xmg_function::operator!=( const xmg_function& other ) const
{
  return !operator==( other );
}

bool xmg_function::operator<( const xmg_function& other ) const
{
  if ( node < other.node )
  {
    return true;
  }
  else if ( node == other.node )
  {
    return !complemented && other.complemented;
  }
  else
  {
    return false;
  }
}

bool xmg_function::operator>( const xmg_function& other ) const
{
  return ( other < *this );
}

xmg_function xmg_function::operator!() const
{
  return xmg_function( node, !complemented );
}

xmg_function xmg_function::operator^( bool value ) const
{
  return xmg_function( node, complemented != value );
}


}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
