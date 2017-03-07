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

#include "aig_utils.hpp"

#include <boost/assign/std/vector.hpp>
#include <boost/format.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/range/algorithm.hpp>

#include <regex>

#include <core/utils/range_utils.hpp>
#include <core/graph/depth.hpp>

using namespace boost::assign;

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

  throw boost::str( boost::format( "cannot find input named '%s'" ) % name );
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
    if ( p.value.second == name )
    {
      return p.index;
    }
  }

  assert( false );

  return 0;
}

void aig_print_stats( const aig_graph& aig, std::ostream& os )
{
  const auto& info = aig_info( aig );
  auto n = info.inputs.size();

  std::string name = info.model_name;
  if ( name.empty() )
  {
    name = "(unnamed)";
  }

  std::vector<aig_node> outputs;
  for ( const auto& output : info.outputs )
  {
    outputs += output.first.node;
  }

  std::vector<unsigned> depths;
  const auto depth = compute_depth( aig, outputs, depths );

  if ( info.latch.empty() )
  {
    os << boost::format( "[i] %20s: i/o = %7d / %7d  and = %7d  lev = %4d" ) % name % n % info.outputs.size() % ( boost::num_vertices( aig ) - n - 1u ) % depth << std::endl;
  }
  else
  {
    os << boost::format( "[i] %20s: i/l/o = %7d / %7d / %7d  and = %7d  lev = %4d" ) % name % n % info.latch.size() % info.outputs.size() % ( boost::num_vertices( aig ) - n - 1u ) % depth << std::endl;
  }
}

std::vector<aig_function> get_children( const aig_graph& aig, const aig_node& node )
{
  std::vector<aig_function> children;

  for ( const auto& edge : boost::make_iterator_range( boost::out_edges( node, aig ) ) )
  {
    children += aig_to_function( aig, edge );
  }

  return children;
}

aig_function make_function( const aig_function& f, bool complemented )
{
  return complemented ? !f : f;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
