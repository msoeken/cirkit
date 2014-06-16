/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2014  University of Bremen
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

#include <fstream>

#include <boost/format.hpp>
#include <boost/range/iterator_range.hpp>

namespace cirkit
{

void write_aiger( const aig_graph& aig, std::ostream& os )
{
  const auto& graph_info = boost::get_property( aig, boost::graph_name );
  const auto& indexmap = get( boost::vertex_name, aig );
  const auto& complementmap = boost::get( boost::edge_complement, aig );

  /* header */
  unsigned _num_inputs = graph_info.inputs.size();
  unsigned _num_outputs = graph_info.outputs.size();
  unsigned _num_vertices = num_vertices( aig ) - 1u;

  os << boost::format( "aag %d %d 0 %d %d" ) % _num_vertices % _num_inputs % _num_outputs % ( _num_vertices - _num_inputs ) << std::endl;

  /* inputs */
  for ( const auto& input : graph_info.inputs )
  {
    os << indexmap[input] << std::endl;
  }

  /* outputs */
  for ( const auto& output : graph_info.outputs )
  {
    os << aig_to_literal( aig, output.first ) << std::endl;
  }

  /* AND gates */
  for ( const auto& node : boost::make_iterator_range( vertices( aig ) ) )
  {
    auto _out_degree = out_degree( node, aig );
    if ( _out_degree )
    {
      assert( _out_degree == 2u );

      os << indexmap[node];

      for ( const auto& edge : boost::make_iterator_range( out_edges( node, aig ) ) )
      {
        os << " " << aig_to_literal( aig, std::make_pair( target( edge, aig ), complementmap[edge] ) );
      }

      os << std::endl;
    }
  }

  /* input names */
  unsigned index = 0u;
  for ( const auto& input : graph_info.inputs )
  {
    os << "i" << index++ << " " << graph_info.node_names.at( input ) << std::endl;
  }

  /* output names */
  index = 0u;
  for ( const auto& output : graph_info.outputs )
  {
    os << "o" << index++ << " " << output.second << std::endl;
  }
}

void write_aiger( const aig_graph& aig, const std::string& filename )
{
  std::filebuf fb;
  fb.open( filename.c_str(), std::ios::out );
  std::ostream os( &fb );
  write_aiger( aig, os );
  fb.close();
}

}

// Local Variables:
// c-basic-offset: 2
// End:
