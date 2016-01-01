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

#include "write_aiger.hpp"

#include <boost/format.hpp>
#include <boost/range/iterator_range.hpp>

#include <fstream>

namespace cirkit
{

void write_aiger( const aig_graph& aig, std::ostream& os, const bool fill_sym_table )
{
  assert( num_vertices( aig ) != 0u && "Uninitialized AIG" );

  const auto& graph_info = boost::get_property( aig, boost::graph_name );
  const auto& indexmap = get( boost::vertex_name, aig );
  const auto& complementmap = boost::get( boost::edge_complement, aig );

  /* header */
  const unsigned _num_inputs = graph_info.inputs.size();
  const unsigned _num_outputs = graph_info.outputs.size();
  const unsigned _num_latches = graph_info.cis.size();
  const unsigned _num_vertices = num_vertices( aig ) - 1u;
  const unsigned _num_gates = _num_vertices - _num_latches - _num_inputs;

  os << boost::format( "aag %d %d %d %d %d" )
    % _num_vertices % _num_inputs % _num_latches % _num_outputs % _num_gates << std::endl;

  /* inputs */
  for ( const auto& input : graph_info.inputs )
  {
    os << indexmap[input] << std::endl;
  }

  /* latches */
  assert( graph_info.cis.size() == graph_info.cos.size() );
  for ( unsigned u = 0u; u < graph_info.cis.size(); ++u )
  {
    os << indexmap[ graph_info.cis[u] ] << ' ' << aig_to_literal( aig, graph_info.cos[u] ) << std::endl;
  }

  /* outputs */
  for ( const auto& output : graph_info.outputs )
  {
    os << aig_to_literal( aig, output.first ) << std::endl;
  }

  /* AND gates */
  for ( const auto& node : boost::make_iterator_range( vertices( aig ) ) )
  {
    const auto _out_degree = out_degree( node, aig );
    if ( _out_degree )
    {
      assert( _out_degree == 2u );

      os << indexmap[node];

      for ( const auto& edge : boost::make_iterator_range( out_edges( node, aig ) ) )
      {
        os << " " << aig_to_literal( aig, { target( edge, aig ), complementmap[edge] } );
      }

      os << std::endl;
    }
  }

  /* input names */
  unsigned index = 0u;
  for ( const auto& input : graph_info.inputs )
  {
    auto it = graph_info.node_names.find( input );
    if ( it != graph_info.node_names.end() )
    {
      os << "i" << index << " " << it->second << std::endl;
    }
    else if ( fill_sym_table )
    {
      os << "i" << index << " input" << index << std::endl;
    }
    ++index;
  }

  /* latch names */
  index = 0u;
  for ( const auto& ci : graph_info.cis )
  {
    auto it = graph_info.node_names.find( ci );
    if ( it != graph_info.node_names.end() )
    {
      os << "l" << index << " " << it->second << std::endl;
    }
    else if ( fill_sym_table )
    {
      os << "l" << index << " latch" << index << std::endl;
    }
    ++index;
  }

  /* output names */
  index = 0u;
  for ( const auto& output : graph_info.outputs )
  {
    const std::string& name = output.second;
    if ( name != "" )
    {
      os << "o" << index << " " << name << std::endl;
    }
    else if ( fill_sym_table )
    {
      os << "o" << index << " output" << index << std::endl;
    }
    ++index;
  }
}

void write_aiger( const aig_graph& aig, const std::string& filename, const bool fill_sym_table )
{
  std::filebuf fb;
  fb.open( filename.c_str(), std::ios::out );
  std::ostream os( &fb );
  write_aiger( aig, os, fill_sym_table );
  fb.close();
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
