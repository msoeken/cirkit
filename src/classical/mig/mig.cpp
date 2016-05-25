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

#include "mig.hpp"

#include <fstream>

#include <boost/format.hpp>
#include <boost/graph/filtered_graph.hpp>
#include <boost/graph/graphviz.hpp>

#include <core/utils/string_utils.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

struct mig_dot_writer
{
  mig_dot_writer( const mig_graph& mig ) : mig( mig )
  {
  }

  /* vertex properties */
  void operator()( std::ostream& os, const mig_node& v )
  {
    string_properties_map_t properties;
    const auto& graph_info = boost::get_property( mig, boost::graph_name );

    /* shape */
    if ( out_degree( v, mig ) == 0u )
    {
      properties["shape"] = "box";
    }

    /* vertex label */
    auto itName = graph_info.node_names.find( v );
    if ( itName != graph_info.node_names.end() )
    {
      properties["label"] = boost::str( boost::format( "\"%s (%d)\"" ) % itName->second % ( v << 1u ) );
    }
    else
    {
      properties["label"] = boost::str( boost::format( "\"%d\"" ) % ( v << 1u ) );
    }

    os << "[" << make_properties_string( properties ) << "]";
  }

  /* edge properties */
  void operator()( std::ostream& os, const mig_edge& e )
  {
    const auto& complementmap = boost::get( boost::edge_complement, mig );

    if ( complementmap[e] )
    {
      os << "[style=dashed]";
    }
  }

  /* graph properties */
  void operator()( std::ostream& os )
  {
    const auto& graph_info = boost::get_property( mig, boost::graph_name );

    /* outputs */
    unsigned index = 0u;
    for ( const auto& o : graph_info.outputs )
    {
      os << "o" << index << "[label=\"" << o.second << "\",shape=box];" << std::endl;
      os << "o" << index << " -> " << o.first.node << " ";
      if ( o.first.complemented )
      {
        os << "[style=dashed]";
      }
      os << ";" << std::endl;
      ++index;
    }
  }

private:
  const mig_graph& mig;
};

struct mig_remove_constant_if_unused
{
  mig_remove_constant_if_unused() {}
  mig_remove_constant_if_unused( const mig_graph& mig ) : mig( &mig ) {}

  template <typename Vertex>
  bool operator()( const Vertex& v ) const
  {
    const auto& graph_info = boost::get_property( *mig, boost::graph_name );

    return ( v != graph_info.constant ) || graph_info.constant_used;
  }

private:
  mig_graph const* mig = nullptr;
};

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

void mig_initialize( mig_graph& mig, const std::string& model_name )
{
  assert( num_vertices( mig ) == 0u );

  auto& info = boost::get_property( mig, boost::graph_name );

  info.model_name = model_name;

  /* create constant node */
  info.constant = add_vertex( mig );

  assert( info.constant == 0u );
}

mig_function mig_get_constant( mig_graph& mig, bool value )
{
  assert( num_vertices( mig ) != 0u && "Uninitialized MIG" );

  auto& info = boost::get_property( mig, boost::graph_name );

  info.constant_used = true;
  return { info.constant, value };
}

bool mig_is_constant_used( const mig_graph& mig )
{
  assert( num_vertices( mig ) != 0u && "Uninitialized MIG" );

  auto& info = boost::get_property( mig, boost::graph_name );

  return info.constant_used;
}

mig_function mig_create_pi( mig_graph& mig, const std::string& name )
{
  assert( num_vertices( mig ) != 0u && "Uninitialized MIG" );

  mig_node node = add_vertex( mig );

  boost::get_property( mig, boost::graph_name ).inputs += node;
  boost::get_property( mig, boost::graph_name ).node_names[node] = name;

  return { node, false };
}

void mig_create_po( mig_graph& mig, const mig_function& f, const std::string& name )
{
  assert( num_vertices( mig ) != 0u && "Uninitialized MIG" );
  boost::get_property( mig, boost::graph_name ).outputs += std::make_pair( f, name );
}

mig_function mig_create_maj( mig_graph& mig, const mig_function& a, const mig_function& b, const mig_function& c )
{
  assert( num_vertices( mig ) != 0u && "Unitialized MIG" );

  auto& info = boost::get_property( mig, boost::graph_name );

  /* Special cases */
  if ( a == b )  { return a; }
  if ( a == c )  { return a; }
  if ( b == c )  { return b; }
  if ( a == !b ) { return c; }
  if ( a == !c ) { return b; }
  if ( b == !c ) { return a; }

  /* If special cases are handled a, b, and c should be all
     different */
  mig_function children[] = {a, b, c};
  std::sort( children, children + 3 );

  const auto key = std::make_tuple( children[0], children[1], children[2] );

  const auto it = info.strash.find( key );
  if ( it != info.strash.end() )
  {
    return it->second;
  }

  mig_node node = add_vertex( mig );
  assert( node == num_vertices( mig ) - 1u );

  const auto& complement = boost::get( boost::edge_complement, mig );

  const auto ea = add_edge( node, children[0].node, mig ).first;
  const auto eb = add_edge( node, children[1].node, mig ).first;
  const auto ec = add_edge( node, children[2].node, mig ).first;

  complement[ea] = children[0].complemented;
  complement[eb] = children[1].complemented;
  complement[ec] = children[2].complemented;

  return info.strash[key] = { node, false };
}

mig_function mig_create_and( mig_graph& mig, const mig_function& a, const mig_function& b )
{
  assert( num_vertices( mig ) != 0u && "Unitialized MIG" );
  return mig_create_maj( mig, mig_get_constant( mig, false ), a, b );
}

mig_function mig_create_or( mig_graph& mig, const mig_function& a, const mig_function& b )
{
  assert( num_vertices( mig ) != 0u && "Unitialized MIG" );
  return mig_create_maj( mig, mig_get_constant( mig, true ), a, b );
}

mig_function mig_create_xor( mig_graph& mig, const mig_function& a, const mig_function& b )
{
  assert( num_vertices( mig ) != 0u && "Unitialized MIG" );
  return mig_create_maj( mig, !a, mig_create_or( mig, a, b ), mig_create_and( mig, a, !b ) );
}

void write_dot( const mig_graph& mig, std::ostream& os, const properties::ptr& settings )
{
  assert( num_vertices( mig ) != 0u && "Uninitialized MIG" );
  boost::filtered_graph<mig_graph, boost::keep_all, mig_remove_constant_if_unused> fg( mig, boost::keep_all(), mig_remove_constant_if_unused( mig ) );

  mig_dot_writer writer( mig );
  boost::write_graphviz( os, fg, writer, writer, writer );
}

void write_dot( const mig_graph& mig, const std::string& filename, const properties::ptr& settings )
{
  assert( num_vertices( mig ) != 0u && "Uninitialized MIG" );
  std::filebuf fb;
  fb.open( filename.c_str(), std::ios::out );
  std::ostream os( &fb );
  write_dot( mig, os, settings );
  fb.close();
}

mig_function mig_to_function( const mig_graph& mig, const mig_edge& edge )
{
  return { target(edge, mig), boost::get( boost::edge_complement, mig )[ edge ] };
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
