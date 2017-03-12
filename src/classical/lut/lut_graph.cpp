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

#include "lut_graph.hpp"

#include <classical/lut/lut_utils.hpp>

#include <boost/format.hpp>

using namespace boost::assign;

namespace cirkit
{

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

unsigned lut_graph_lut_count( const lut_graph_t& g )
{
  const auto& type = boost::get( boost::vertex_lut_type, g );

  return boost::count_if( boost::vertices( g ), [&type]( lut_vertex_t v ) { return type[v] == lut_type_t::internal; } );
}

lut_graph::lut_graph( const std::string& name )
  : gnd( add_vertex( g ) )
  , vdd( add_vertex( g ) )
  , _name( name )
  , _names( boost::get( boost::vertex_name, g ) )
  , _types( boost::get( boost::vertex_lut_type, g ) )
  , _luts( boost::get( boost::vertex_lut, g ) )
{
  assert( gnd == 0 );
  assert( vdd == 1 );
  _types[gnd] = lut_type_t::gnd;
  _names[gnd] = "gnd";

  _types[vdd] = lut_type_t::vdd;
  _names[vdd] = "vdd";
}

void lut_graph::compute_fanout() const
{
  fanout.update( [this]() { return precompute_in_degrees( g ); } );
}

void lut_graph::compute_parents() const
{
  parentss.update( [this]() { return precompute_ingoing_vertices( g ); } );
}

void lut_graph::compute_levels() const
{
  levels.update( [this]() { return lut_compute_levels( *this ); } );
}

void lut_graph::compute_sections() const
{
  sections.update( [this]() { return lut_compute_sections( *this ); } );
}

lut_graph::node_t lut_graph::get_constant( bool value ) const
{
  return value ? vdd : gnd;
}

lut_graph::node_t lut_graph::create_pi( const std::string& name )
{
  const auto node = add_vertex( g );
  _names[node] = name;
  _types[node] = lut_type_t::pi;
  _input_to_id.insert( {node, _inputs.size()} );
  _inputs.push_back( {node, name} );
  return node;
}

void lut_graph::create_po( const node_t& n, const std::string& name )
{
  const auto node = add_vertex( g );
  _names[node] = name;
  _types[node] = lut_type_t::po;
  add_edge( node, n, g );
  _outputs.push_back( {n, name} );
}

void lut_graph::delete_po( unsigned index )
{
  if ( index < _outputs.size() )
  {
    _outputs.erase( _outputs.begin() + index );
  }
}

lut_graph::node_t lut_graph::create_lut( const std::string& lut, const std::vector<node_t>& ops, const std::string& name )
{
  const auto node = add_vertex( g );
  _names[node] = name == "" ? boost::str( boost::format("n%d") % node ) : name;
  _luts[node] = lut;
  _types[node] = lut_type_t::internal;
  for ( const auto& op : ops )
  {
    add_edge( node, op, g );
  }
  ++_num_lut;
  return node;
}

unsigned lut_graph::fanin_count( const node_t& n ) const
{
  return boost::out_degree( n, g );
}

unsigned lut_graph::fanout_count( const node_t& n ) const
{
  return (*fanout)[n];
}

const std::vector<lut_graph::node_t>& lut_graph::parents( const node_t& n ) const
{
  return (*parentss)[n];
}

const boost::dynamic_bitset<>& lut_graph::section( const node_t& n ) const
{
  return (*sections)[n];
}

unsigned lut_graph::level( const node_t& n ) const
{
  return (*levels)[n];
}

bool lut_graph::is_input( const node_t& n ) const
{
  return fanin_count( n ) == 0u;
}

bool lut_graph::is_lut( const node_t& n ) const
{
  return _types[n] == lut_type_t::internal;
}

const std::string& lut_graph::name() const
{
  return _name;
}

void lut_graph::set_name( const std::string& name )
{
  _name = name;
}

std::size_t lut_graph::size() const
{
  return num_vertices( g );
}

unsigned lut_graph::num_gates() const
{
  return _num_lut;
}

const lut_graph::graph_t& lut_graph::graph() const
{
  return g;
}

lut_graph::graph_t& lut_graph::graph()
{
  return g;
}

const lut_graph::input_vec_t& lut_graph::inputs() const
{
  return _inputs;
}

const lut_graph::output_vec_t& lut_graph::outputs() const
{
  return _outputs;
}

lut_graph::input_vec_t& lut_graph::inputs()
{
  return _inputs;
}

lut_graph::output_vec_t& lut_graph::outputs()
{
  return _outputs;
}

const std::string& lut_graph::input_name( const node_t& n ) const
{
  return _inputs[_input_to_id.at( n )].second;
}

const unsigned lut_graph::input_index( const node_t& n ) const
{
  return _input_to_id.at( n );
}

lut_graph::vertex_range_t lut_graph::nodes() const
{
  const auto v = boost::vertices( g );
  return ranges::make_iterator_range( v.first, v.second );
}

lut_graph::edge_range_t lut_graph::edges() const
{
  const auto e = boost::edges( g );
  return ranges::make_iterator_range( e.first, e.second );
}

std::vector<lut_graph::node_t> lut_graph::children( const node_t& v ) const
{
  std::vector<node_t> c;
  for ( const auto& e : boost::make_iterator_range( boost::out_edges( v, g ) ) )
  {
    c.push_back( boost::target( e, g ) );
  }
  return c;
}

std::vector<lut_graph::node_t> lut_graph::topological_nodes() const
{
  std::vector<node_t> top( num_vertices( g ) );
  boost::topological_sort( g, top.begin() );
  return top;
}

void lut_graph::init_refs() const
{
  compute_fanout();
  ref_count = *fanout;
}

unsigned lut_graph::get_ref( const lut_vertex_t& n ) const
{
  return ref_count[n];
}

unsigned lut_graph::inc_ref( const lut_vertex_t& n ) const
{
  return ref_count[n]++;
}

unsigned lut_graph::dec_ref( const lut_vertex_t& n ) const
{
  assert( ref_count[n] > 0 );
  return --ref_count[n];
}

void lut_graph::inc_output_refs() const
{
  for ( const auto& output : outputs() )
  {
    ++ref_count[output.first];
  }
}

void lut_graph::init_marks() const
{
  marks.resize( size() );
  marks.reset();
}

bool lut_graph::is_marked( const lut_vertex_t& n ) const
{
  return n < marks.size() && marks[n];
}

void lut_graph::mark( const lut_vertex_t& n ) const
{
  if ( n < marks.size() )
  {
    marks.set( n );
  }
}

void lut_graph::mark_as_modified() const
{
  fanout.make_dirty();
  parentss.make_dirty();
  levels.make_dirty();
  sections.make_dirty();
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
