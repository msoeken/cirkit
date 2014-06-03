/* RevKit: A Toolkit for Reversible Circuit Design (www.revkit.org)
 * Copyright (C) 2009-2013  The RevKit Developers <revkit@informatik.uni-bremen.de>
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

#include "aig.hpp"

#include <boost/algorithm/string/join.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/format.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/range/adaptors.hpp>

namespace revkit
{

using namespace boost::assign;

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

typedef std::map<std::string, std::string> properties_map_t;

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

std::string make_string( const properties_map_t& properties )
{
  using boost::adaptors::transformed;

  return boost::join( properties | transformed( []( const std::pair<std::string, std::string>& p ) { return boost::str( boost::format( "%s=%s" ) % p.first % p.second ); } ), "," );
}

struct aig_dot_writer
{
  aig_dot_writer( const aig_graph& aig ) : aig( aig ) {}

  /* vertex properties */
  void operator()( std::ostream& os, const aig_node& v )
  {
    properties_map_t properties;
    const auto& graph_info = boost::get_property( aig, boost::graph_name );
    const auto& namemap = boost::get( boost::vertex_name, aig );

    if ( out_degree( v, aig ) == 0u )
    {
      properties["shape"] = "box";
    }

    auto itName = graph_info.node_names.find( v );
    if ( itName != graph_info.node_names.end() )
    {
      properties["label"] = boost::str( boost::format( "\"%s (%d)\"" ) % itName->second % namemap[v] );
    }
    else
    {
      properties["label"] = boost::str( boost::format( "\"%s\"" ) % namemap[v] );
    }

    os << "[" << make_string( properties ) << "]";
  }

  /* edge properties */
  void operator()( std::ostream& os, const aig_edge& e )
  {
    const auto& polaritymap = boost::get( boost::edge_polarity, aig );

    if ( !polaritymap[e] )
    {
      os << "[style=dashed]";
    }
  }

  /* graph properties */
  void operator()( std::ostream& os )
  {
    const auto& graph_info = boost::get_property( aig, boost::graph_name );

    unsigned index = 0u;
    for ( const auto& o : graph_info.outputs )
    {
      os << "o" << index << "[label=\"" << o.second << "\",shape=box];" << std::endl;
      os << "o" << index << " -> " << o.first.first << " ";
      if ( !o.first.second )
      {
        os << "[style=dashed]";
      }
      os << ";" << std::endl;
      ++index;
    }
  }

private:
  const aig_graph& aig;
};

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

void aig_initialize( aig_graph& aig )
{
  /* create constant node */
  aig_node node = add_vertex( aig );
  boost::get( boost::vertex_name, aig )[node] = 0u;
}

aig_function aig_get_constant( aig_graph& aig, bool value )
{
  return std::make_pair( 0u, value );
}

aig_function aig_create_pi( aig_graph& aig, const std::string& name )
{
  aig_node node = add_vertex( aig );
  boost::get( boost::vertex_name, aig )[node] = 2u * ( num_vertices( aig ) - 1u );

  boost::get_property( aig, boost::graph_name ).inputs += node;
  boost::get_property( aig, boost::graph_name ).node_names[node] = name;

  return std::make_pair( node, true );
}

void aig_create_po( aig_graph& aig, const aig_function& f, const std::string& name )
{
  boost::get_property( aig, boost::graph_name ).outputs += std::make_pair( f, name );
}

aig_function aig_create_and( aig_graph& aig, const aig_function& left, const aig_function& right )
{
  auto& info = boost::get_property( aig, boost::graph_name );

  /* structural hashing */
  bool in_order = left.first < right.first;
  auto key = std::make_pair( in_order ? left : right, in_order ? right : left );
  auto it = info.strash.find( key );
  if ( it != info.strash.end() )
  {
    return it->second;
  }

  /* create node and connect to children */
  aig_node node = add_vertex( aig );
  boost::get( boost::vertex_name, aig )[node] = 2u * ( num_vertices( aig ) - 1u );

  aig_edge le = add_edge( node, left.first, aig ).first;
  boost::get( boost::edge_polarity, aig )[le] = left.second;
  aig_edge re = add_edge( node, right.first, aig ).first;
  boost::get( boost::edge_polarity, aig )[re] = right.second;

  return info.strash[key] = std::make_pair( node, true );
}

aig_function aig_create_or( aig_graph& aig, const aig_function& left, const aig_function& right )
{
  return !aig_create_and( aig, !left, !right );
}

aig_function aig_create_ite( aig_graph& aig, const aig_function& cond, const aig_function& t, const aig_function& e )
{
  return aig_create_or( aig, aig_create_and( aig, cond, t ), aig_create_and( aig, !cond, e ) );
}

void write_dot( std::ostream& os, const aig_graph& aig )
{
  auto indexmap = get( boost::vertex_name, aig );

  aig_dot_writer writer( aig );
  boost::write_graphviz( os, aig, writer, writer, writer );
}

aig_function operator!( const aig_function& f )
{
  return std::make_pair( f.first, !f.second );
}


}

// Local Variables:
// c-basic-offset: 2
// End:
