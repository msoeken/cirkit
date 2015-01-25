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

#include "aig_utils.hpp"

#include <boost/format.hpp>
#include <boost/range/algorithm.hpp>

#include <regex>

#include <core/utils/range_utils.hpp>

namespace cirkit
{

namespace detail
{

void update_msb_map( std::map< std::string, unsigned >& msb,
                     const std::string& name, const unsigned idx )
{
  auto it = msb.find( name );
  if ( it != msb.end() )
  {
    if ( idx > it->second )
    {
      it->second = idx;
    }
  }
  else
  {
    msb.insert( {name,idx} );
  }
}

} /* detail */

bool split_name( std::string& na, unsigned& idx, const std::string& name )
{
  std::regex re( "([a-zA-Z0-9_]+)\\[([0-9]+)\\]" );
  std::smatch match;
  if ( std::regex_match(name, match, re) )
  {
    na = match[1].str();
    unsigned n;
    sscanf( match[2].str().c_str(), "%u", &n);
    idx = n;
    return true;
  }

  return false;
}

void aig_mirror_word_names( aig_graph& aig )
{
  assert( num_vertices( aig ) != 0u && "Uninitialized AIG" );

  auto& graph_info = boost::get_property( aig, boost::graph_name );

  std::map< std::string, unsigned > msb;

  /* compute the msb for each input */
  for ( const auto& input : graph_info.inputs )
  {
    auto it = graph_info.node_names.find( input );
    if ( it != graph_info.node_names.end() )
    {
      std::string na;
      unsigned idx;
      if ( split_name( na, idx, it->second ) ) {
        detail::update_msb_map( msb, na, idx );
      }
    }
  }

  /* mirror bits: update all map entries */
  for ( const auto& input : graph_info.inputs )
  {
    auto it = graph_info.node_names.find( input );
    if ( it != graph_info.node_names.end() )
    {
      std::string na;
      unsigned idx;
      if ( split_name( na, idx, it->second ) ) {
        auto msb_it = msb.find( na );
        assert( msb_it != msb.end() );
        it->second = ( boost::format("%s[%u]") % na % (msb_it->second - idx) ).str();
      }
    }
  }
}

aig_node aig_node_by_name( const aig_graph& aig, const std::string& name )
{
  auto& info = boost::get_property( aig, boost::graph_name );
  return aig_node_by_name( info, name );
}

aig_node aig_node_by_name( const aig_graph_info& info, const std::string& name )
{
  for ( const auto& p : info.node_names )
  {
    if ( p.second == name )
    {
      return p.first;
    }
  }

  assert( false );
}

unsigned aig_input_index( const aig_graph& aig, const aig_node& input )
{
  auto& info = boost::get_property( aig, boost::graph_name );
  return aig_input_index( info, input );
}

unsigned aig_input_index( const aig_graph_info& info, const aig_node& input )
{
  return std::distance( info.inputs.begin(), boost::find( info.inputs, input ) );
}

unsigned aig_output_index( const aig_graph& aig, const std::string& name )
{
  auto& info = boost::get_property( aig, boost::graph_name );
  return aig_output_index( info, name );
}

unsigned aig_output_index( const aig_graph_info& info, const std::string& name )
{
  for ( const auto& p : index( info.outputs ) )
  {
    if ( p.second.second == name )
    {
      return p.first;
    }
  }

  assert( false );
}


}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
