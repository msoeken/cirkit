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

#include "aig.hpp"

#include <fstream>

#include <boost/algorithm/string/join.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/format.hpp>
#include <boost/graph/filtered_graph.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/range/adaptors.hpp>

namespace cirkit
{

using namespace boost::assign;

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

typedef std::map<std::string, std::string> properties_map_t;

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

std::string make_string( const properties_map_t& properties, const std::string& sep = "," )
{
  using boost::adaptors::transformed;

  return boost::join( properties | transformed( []( const std::pair<std::string, std::string>& p ) { return boost::str( boost::format( "%s=%s" ) % p.first % p.second ); } ), sep );
}

struct aig_dot_writer
{
  aig_dot_writer( const aig_graph& aig ) : aig( aig ) {}

  /* vertex properties */
  void operator()( std::ostream& os, const aig_node& v )
  {
    properties_map_t properties;
    const auto& graph_info    = boost::get_property( aig, boost::graph_name );
    const auto& namemap       = boost::get( boost::vertex_name, aig );
    const auto& annotationmap = boost::get( boost::vertex_annotation, aig );

    /* shape */
    if ( out_degree( v, aig ) == 0u )
    {
      properties["shape"] = "box";
    }

    /* vertex annotations */
    const auto& m = annotationmap[v];
    std::string annotations = m.empty() ? "" : boost::str( boost::format( "<br/><font point-size=\"10\" color=\"blue\">%s</font>" ) % make_string( m, "<br/>" ) );
    std::string quotes = m.empty() ? "\"\"" : "<>"; /* if we do not use HTML, we should use normal quotes */

    /* vertex label */
    auto itName = graph_info.node_names.find( v );
    if ( itName != graph_info.node_names.end() )
    {
      properties["label"] = boost::str( boost::format( "%c%s (%d)%s%c" ) % quotes[0] % itName->second % namemap[v] % annotations % quotes[1] );
    }
    else
    {
      properties["label"] = boost::str( boost::format( "%c%s%s%c" ) % quotes[0] % namemap[v] % annotations % quotes[1] );
    }

    os << "[" << make_string( properties ) << "]";
  }

  /* edge properties */
  void operator()( std::ostream& os, const aig_edge& e )
  {
    const auto& complementmap = boost::get( boost::edge_complement, aig );

    if ( complementmap[e] )
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
      if ( o.first.second )
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

struct remove_constant_if_unused
{
  remove_constant_if_unused() {}
  remove_constant_if_unused( const aig_graph& aig ) : aig( &aig ) {}

  template <typename Vertex>
  bool operator()( const Vertex& v ) const
  {
    const auto& graph_info = boost::get_property( *aig, boost::graph_name );

    return ( v != graph_info.constant ) || graph_info.constant_used;
  }

private:
  aig_graph const* aig = nullptr;
};

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

void aig_initialize( aig_graph& aig, const std::string& model_name )
{
  assert( num_vertices( aig ) == 0u );

  auto& info = boost::get_property( aig, boost::graph_name );

  info.model_name = model_name;

  /* create constant node */
  info.constant = add_vertex( aig );
  boost::get( boost::vertex_name, aig )[info.constant] = 0u;
}

aig_function aig_get_constant( aig_graph& aig, bool value )
{
  assert( num_vertices( aig ) != 0u && "Uninitialized AIG" );

  auto& info = boost::get_property( aig, boost::graph_name );

  info.constant_used = true;
  return std::make_pair( info.constant, value );
}

bool aig_is_constant_used( const aig_graph& aig )
{
  assert( num_vertices( aig ) != 0u && "Uninitialized AIG" );

  auto& info = boost::get_property( aig, boost::graph_name );

  return info.constant_used;
}

aig_function aig_create_pi( aig_graph& aig, const std::string& name )
{
  assert( num_vertices( aig ) != 0u && "Uninitialized AIG" );

  aig_node node = add_vertex( aig );
  boost::get( boost::vertex_name, aig )[node] = 2u * ( num_vertices( aig ) - 1u );

  boost::get_property( aig, boost::graph_name ).inputs += node;
  boost::get_property( aig, boost::graph_name ).node_names[node] = name;

  return std::make_pair( node, false );
}

void aig_create_po( aig_graph& aig, const aig_function& f, const std::string& name )
{
  assert( num_vertices( aig ) != 0u && "Uninitialized AIG" );
  boost::get_property( aig, boost::graph_name ).outputs += std::make_pair( f, name );
}

aig_function aig_create_ci( aig_graph& aig, const std::string& name )
{
  assert( num_vertices( aig ) != 0u && "Uninitialized AIG" );

  aig_node node = add_vertex( aig );
  boost::get( boost::vertex_name, aig )[node] = 2u * ( num_vertices( aig ) - 1u );

  boost::get_property( aig, boost::graph_name ).cis += node;
  boost::get_property( aig, boost::graph_name ).node_names[node] = name;

  return std::make_pair( node, false );
}

void aig_create_co( aig_graph& aig, const aig_function& f )
{
  assert( num_vertices( aig ) != 0u && "Uninitialized AIG" );
  boost::get_property( aig, boost::graph_name ).cos += f;
}

aig_function aig_create_and( aig_graph& aig, const aig_function& left, const aig_function& right )
{
  assert( num_vertices( aig ) != 0u && "Uninitialized AIG" );

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
  assert( 2u * node == 2u * ( num_vertices( aig ) - 1u ) );

  aig_edge le = add_edge( node, left.first, aig ).first;
  boost::get( boost::edge_complement, aig )[le] = left.second;
  aig_edge re = add_edge( node, right.first, aig ).first;
  boost::get( boost::edge_complement, aig )[re] = right.second;

  return info.strash[key] = std::make_pair( node, false );
}

aig_function aig_create_nand( aig_graph& aig, const aig_function& left, const aig_function& right )
{
  assert( num_vertices( aig ) != 0u && "Uninitialized AIG" );
  return !aig_create_and( aig, left, right );
}

aig_function aig_create_or( aig_graph& aig, const aig_function& left, const aig_function& right )
{
  assert( num_vertices( aig ) != 0u && "Uninitialized AIG" );
  return !aig_create_and( aig, !left, !right );
}

aig_function aig_create_nor( aig_graph& aig, const aig_function& left, const aig_function& right )
{
  assert( num_vertices( aig ) != 0u && "Uninitialized AIG" );
  return aig_create_and( aig, !left, !right );
}

aig_function aig_create_xor( aig_graph& aig, const aig_function& left, const aig_function& right )
{
  assert( num_vertices( aig ) != 0u && "Uninitialized AIG" );
  return aig_create_or( aig, aig_create_and( aig, !left, right), aig_create_and( aig, left, !right ) );
}

aig_function aig_create_ite( aig_graph& aig, const aig_function& cond, const aig_function& t, const aig_function& e )
{
  assert( num_vertices( aig ) != 0u && "Uninitialized AIG" );
  return aig_create_or( aig, aig_create_and( aig, cond, t ), aig_create_and( aig, !cond, e ) );
}

aig_function aig_create_implies( aig_graph& aig, const aig_function& a, const aig_function& b )
{
  assert( num_vertices( aig ) != 0u && "Uninitialized AIG" );
  return aig_create_or( aig, !a, b );
}

aig_function aig_create_nary_and( aig_graph& aig, const std::vector< aig_function >& v )
{
  assert( num_vertices( aig ) != 0u && "Uninitialized AIG" );

  const auto size = v.size();
  assert( size >= 2 );

  aig_function result;
  if ( size >= 2 )
  {
    result = aig_create_and( aig, v[0], v[1] );
  }
  for ( unsigned u = 2u; u < size; ++u )
  {
    result = aig_create_and( aig, result, v[u] );
  }
  return result;
}

aig_function aig_create_nary_nand( aig_graph& aig, const std::vector< aig_function >& v )
{
  return !aig_create_nary_and( aig, v );
}

aig_function aig_create_nary_or( aig_graph& aig, const std::vector< aig_function >& v )
{
  assert( num_vertices( aig ) != 0u && "Uninitialized AIG" );

  const auto size = v.size();
  assert( size >= 2 );

  aig_function result;
  if ( size >= 2 )
  {
    result = aig_create_or( aig, v[0], v[1] );
  }
  for ( unsigned u = 2u; u < size; ++u )
  {
    result = aig_create_or( aig, result, v[u] );
  }
  return result;
}

aig_function aig_create_nary_nor( aig_graph& aig, const std::vector< aig_function >& v )
{
  return !aig_create_nary_or( aig, v );
}

aig_function aig_create_nary_xor( aig_graph& aig, const std::vector< aig_function >& v )
{
  assert( num_vertices( aig ) != 0u && "Uninitialized AIG" );

  const auto size = v.size();
  assert( size >= 2 );

  aig_function result;
  if ( size >= 2 )
  {
    result = aig_create_xor( aig, v[0], v[1] );
  }
  for ( unsigned u = 2u; u < size; ++u )
  {
    result = aig_create_xor( aig, result, v[u] );
  }
  return result;
}

aig_function aig_create_maj( aig_graph& aig, const aig_function& a, const aig_function& b, const aig_function& c )
{
  assert( num_vertices( aig ) != 0u && "Uninitialized AIG" );
  return aig_create_or( aig, aig_create_or( aig, aig_create_and( aig, a, b ), aig_create_and( aig, a, c ) ), aig_create_and( aig, b, c ) );
}

aig_function aig_create_lat( aig_graph& aig, const aig_function& in, const std::string& name )
{
  assert( num_vertices( aig ) != 0u && "Uninitialized AIG" );

  auto& info = boost::get_property( aig, boost::graph_name );

  /* structural hashing */
  auto it = info.latch.find( in );
  if ( it != info.latch.end() )
  {
    return it->second;
  }

  /* create corresponding ci and co nodes */
  aig_node node = aig_create_ci( aig, name ).first;
  aig_create_co( aig, in );

  return info.latch[in] = std::make_pair( node, false );
}

void write_dot( const aig_graph& aig, std::ostream& os )
{
  assert( num_vertices( aig ) != 0u && "Uninitialized AIG" );
  boost::filtered_graph<aig_graph, boost::keep_all, remove_constant_if_unused> fg( aig, boost::keep_all(), remove_constant_if_unused( aig ) );

  aig_dot_writer writer( aig );
  boost::write_graphviz( os, fg, writer, writer, writer );
}

void write_dot( const aig_graph& aig, const std::string& filename )
{
  assert( num_vertices( aig ) != 0u && "Uninitialized AIG" );
  std::filebuf fb;
  fb.open( filename.c_str(), std::ios::out );
  std::ostream os( &fb );
  write_dot( aig, os );
  fb.close();
}

unsigned aig_to_literal( const aig_graph& aig, const aig_function& f )
{
  assert( num_vertices( aig ) != 0u && "Uninitialized AIG" );
  const auto& indexmap = get( boost::vertex_name, aig );

  return indexmap[f.first] + ( f.second ? 1u : 0u );
}

unsigned aig_to_literal( const aig_graph& aig, const aig_node& node )
{
  assert( num_vertices( aig ) != 0u && "Uninitialized AIG" );
  return boost::get( boost::vertex_name, aig )[node];
}

unsigned aig_to_literal( const aig_function& f )
{
  return 2*f.first + (f.second ? 1u : 0u);
}

aig_function operator!( const aig_function& f )
{
  return std::make_pair( f.first, !f.second );
}

std::ostream& operator<<( std::ostream& os, const aig_function &f )
{
  return os << aig_to_literal(f);
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
