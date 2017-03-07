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
