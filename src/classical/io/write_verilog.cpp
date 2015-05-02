/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
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

#include "write_verilog.hpp"

#include <fstream>

#include <boost/algorithm/string/join.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/format.hpp>
#include <boost/range/algorithm.hpp>

#include <classical/io/io_utils_p.hpp>

using namespace boost::assign;
using boost::format;

namespace cirkit
{

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

std::string remove_brackets( const std::string& s )
{
  std::string sc = s;
  size_t n = 0;

  while ( ( n = sc.find( '[', n ) ) != std::string::npos )
  {
    auto n2 = sc.find( ']', n );
    sc[n] = sc[n2] = '_';
    n = n2;
  }

  return sc;
}

void create_possible_inverter( std::ostream& os, const aig_function& f, std::vector<aig_node>& inverter_created, const aig_graph& aig )
{
  if ( f.complemented && boost::find( inverter_created, f.node ) == inverter_created.end() )
  {
    os << format( "not %1%_inv( %1%_inv, %1%" ) % get_node_name_processed( f.node, aig, &remove_brackets ) << std::endl;
    inverter_created += f.node;
  }
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

void write_verilog( const aig_graph& aig, std::ostream& os )
{
  const auto& aig_info = boost::get_property( aig, boost::graph_name );

  std::vector<std::string> inputs, outputs;
  std::vector<aig_node> inverter_created;

  /* Inputs */
  for ( const auto& v : aig_info.inputs )
  {
    inputs += remove_brackets( aig_info.node_names.find( v )->second );
  }

  /* Outputs */
  for ( const auto& v : aig_info.outputs )
  {
    outputs += remove_brackets( v.second );
  }

  os << format( "module top(%s, %s);" ) % boost::join( inputs, ", " ) % boost::join( outputs, ", " ) << std::endl
     << format( "input %s;" ) % boost::join( inputs, ", " ) << std::endl
     << format( "output %s;" ) % boost::join( outputs, ", " ) << std::endl;

  /* AND gates */
  for ( const auto& v : boost::make_iterator_range( boost::vertices( aig ) ) )
  {
    /* skip outputs */
    if ( boost::out_degree( v, aig ) == 0u ) continue;

    auto operands = get_operands( v, aig );
    create_possible_inverter( os, operands.first, inverter_created, aig );
    create_possible_inverter( os, operands.second, inverter_created, aig );

    os << format( "and %1%( %1%, %2%%3%, %4%%5% )" ) % get_node_name_processed( v, aig, &remove_brackets )
      % get_node_name_processed( operands.first.node, aig, &remove_brackets ) % ( operands.first.complemented ? "_inv" : "" )
      % get_node_name_processed( operands.second.node, aig, &remove_brackets ) % ( operands.second.complemented ? "_inv" : "" ) << std::endl;
  }

  /* Output functions */
  for ( const auto& v : aig_info.outputs )
  {
    os << format( "%1% %2%( %2%, %3% )" ) % ( v.first.complemented ? "not" : "buf" )
                                          % remove_brackets( v.second )
                                          % get_node_name_processed( v.first.node, aig, &remove_brackets )
       << std::endl;
  }

  os << "endmodule" << std::endl;
}

void write_verilog( const aig_graph& aig, const std::string& filename )
{
  std::ofstream os( filename.c_str(), std::ofstream::out );
  write_verilog( aig, os );
  os.close();
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
