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

#include "netlist_graphs.hpp"

#include <boost/algorithm/string/join.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/format.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/iterator_range.hpp>

#include <core/utils/graph_utils.hpp>

using namespace boost::assign;

namespace cirkit
{

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

std::string type_to_name( gate_type_t type )
{
  switch ( type )
  {
  case gate_type_t::gnd:  return "GND";
  case gate_type_t::inv:  return "INV";
  case gate_type_t::buf:  return "BUF";
  case gate_type_t::_and: return "AND";
  case gate_type_t::_or:  return "OR";
  case gate_type_t::nand: return "NAND";
  case gate_type_t::nor:  return "NOR";
  case gate_type_t::_xor: return "XOR";
  case gate_type_t::xnor: return "XNOR";
  case gate_type_t::mux:  return "MUX";
  case gate_type_t::fadd: return "FADD";
  default: assert( 0 );
  }
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

void write_simple_fanout_graph( std::ostream& os, const simple_fanout_graph_t& g )
{
  using boost::format;
  using boost::str;

  std::vector<std::string> vinputs, voutputs;
  const auto& vertex_name = boost::get( boost::vertex_name, g );
  const auto& gate_type   = boost::get( boost::vertex_gate_type, g );
  const auto& edge_name   = boost::get( boost::edge_name, g );

  /* name_or_id */
  auto name_or_id = [&vertex_name]( const simple_fanout_vertex_t& v ) {
    const auto& n = vertex_name[v];
    return n.empty() ? str( format( "n%d" ) % v ) : n;
  };

  for ( const auto& v : boost::make_iterator_range( vertices( g ) ) )
  {
    if ( gate_type[v] == gate_type_t::pi ) { vinputs += str( format( "input %s" ) % name_or_id( v ) ); }
    else if ( gate_type[v] == gate_type_t::po ) { voutputs += str( format( "output %s" ) % name_or_id( v ) ); }
  }

  auto inputs = boost::join( vinputs, ", " );
  auto outputs = boost::join( voutputs, ", " );

  os << format( "module top(%s, %s);" ) % inputs % outputs << std::endl << std::endl;

  auto in_edges = precompute_ingoing_edges( g );

  std::vector<simple_fanout_vertex_t> topo;
  boost::topological_sort( g, std::back_inserter( topo ) );
  boost::reverse( topo );
  for ( const auto& v : topo )
  {
    if ( gate_type[v] == gate_type_t::pi || gate_type[v] == gate_type_t::fanout || gate_type[v] == gate_type_t::internal )
    {
      continue;
    }

    if ( gate_type[v] == gate_type_t::po )
    {
      assert( in_edges[v].size() <= 1u );
      if ( in_edges[v].size() == 0u ) continue;
      auto s = boost::source( in_edges[v].front(), g );
      if ( gate_type[s] == gate_type_t::fanout ) { s = boost::source( in_edges[s].front(), g ); }
      os << format( "VERIFIC_BUF g%s(.o(%s), .i(%s));" ) % name_or_id( v ) % name_or_id( v ) % name_or_id( s ) << std::endl;
    }
    else
    {
      std::vector<std::string> ports;
      for ( const auto& e : in_edges[v] )
      {
        auto s = boost::source( e, g );
        if ( gate_type[s] == gate_type_t::fanout ) { s = boost::source( in_edges[s].front(), g ); }
        ports += str( format( ".%s( %s )" ) % edge_name[e].second % name_or_id( s ) );
      }

      if ( boost::out_degree( v, g ) == 1u )
      {
        ports += str( format( ".%s( %s )" ) % edge_name[*boost::out_edges( v, g ).first].first % name_or_id( v ) );
      }
      os << format( "VERIFIC_%s g%s(%s);" ) % type_to_name( gate_type[v] ) % name_or_id( v ) % boost::join( ports, ", " ) << std::endl;
    }
  }

  os << "endmodule" << std::endl;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
