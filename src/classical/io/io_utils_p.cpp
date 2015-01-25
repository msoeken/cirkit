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

#include "io_utils_p.hpp"

#include <boost/format.hpp>
#include <boost/range/iterator_range.hpp>

namespace cirkit
{

std::pair<aig_function, aig_function> get_operands( const aig_node& v, const aig_graph& aig )
{
  const auto& complement = boost::get( boost::edge_complement, aig );
  std::pair<aig_function, aig_function> p;

  bool first = true;
  for ( const auto& e : boost::make_iterator_range( boost::out_edges( v, aig ) ) )
  {
    ( first ? p.first : p.second ) = {boost::target( e, aig ), complement[e]};
    first = false;
  }

  return p;
}

std::string get_node_name( const aig_node& v, const aig_graph& aig, const std::string& prefix )
{
  const auto& aig_info = boost::get_property( aig, boost::graph_name );
  if ( boost::out_degree( v, aig ) == 0u && v != aig_info.constant )
  {
    return boost::str( boost::format( "%s%s" ) % prefix % aig_info.node_names.find( v )->second );
  }
  else
  {
    return boost::str( boost::format( "%sn%d" ) % prefix % boost::get( boost::vertex_name, aig )[v] );
  }
}

std::string get_node_name_processed( const aig_node& v, const aig_graph& aig, const std::function<std::string(const std::string&)>& f, const std::string& prefix )
{
  const auto& aig_info = boost::get_property( aig, boost::graph_name );
  if ( boost::out_degree( v, aig ) == 0u && v != aig_info.constant )
  {
    return boost::str( boost::format( "%s%s" ) % prefix % f( aig_info.node_names.find( v )->second ) );
  }
  else
  {
    return boost::str( boost::format( "%sn%d" ) % prefix % boost::get( boost::vertex_name, aig )[v] );
  }
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
