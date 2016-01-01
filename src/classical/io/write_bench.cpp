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

#include "write_bench.hpp"

#include <fstream>

#include <boost/format.hpp>
#include <boost/range/iterator_range.hpp>

#include <classical/io/io_utils_p.hpp>

namespace cirkit
{

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

void write_bench( const aig_graph& aig, std::ostream& os, const write_bench_settings& settings )
{
  const auto& aig_info = boost::get_property( aig, boost::graph_name );
  const auto& name = boost::get( boost::vertex_name, aig );

  /* Inputs */
  if ( settings.write_input_declarations )
  {
    for ( const auto& v : aig_info.inputs )
    {
      os << boost::format( "INPUT(%s%s)" ) % settings.prefix % aig_info.node_names.find( v )->second << std::endl;
    }
  }

  /* Outputs */
  if ( settings.write_output_declarations )
  {
    for ( const auto& v : aig_info.outputs )
    {
      os << boost::format( "OUTPUT(%s%s)" ) % settings.prefix % v.second << std::endl;
    }
  }

  /* Constant */
  if ( aig_info.constant_used )
  {
    os << boost::format( "%sn%d = gnd" ) % settings.prefix % name[aig_info.constant] << std::endl;
  }

  /* AND gates */
  for ( const auto& v : boost::make_iterator_range( boost::vertices( aig ) ) )
  {
    /* skip outputs */
    if ( boost::out_degree( v, aig ) == 0u ) continue;

    auto operands = get_operands( v, aig );
    unsigned lut = 0x8;
    if ( operands.first.complemented )  lut >>= 0x1;
    if ( operands.second.complemented ) lut >>= 0x2;

    os << boost::format( "%sn%d = LUT 0x%x ( %s, %s )" ) % settings.prefix % name[v] % lut % get_node_name( operands.first.node, aig, settings.prefix ) % get_node_name( operands.second.node, aig, settings.prefix ) << std::endl;
  }

  /* Output functions */
  for ( const auto& v : aig_info.outputs )
  {
    unsigned lut = v.first.complemented ? 0x1 : 0x2;
    os << boost::format( "%s%s = LUT 0x%x ( %s )" ) % settings.prefix % v.second % lut % get_node_name( v.first.node, aig, settings.prefix ) << std::endl;
  }
}

void write_bench( const aig_graph& aig, const std::string& filename, const write_bench_settings& settings )
{
  std::ofstream os( filename.c_str(), std::ofstream::out );
  write_bench( aig, os, settings );
  os.close();
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
