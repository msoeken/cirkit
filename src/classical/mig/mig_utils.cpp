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

#include "mig_utils.hpp"

#include <boost/assign/std/vector.hpp>
#include <boost/format.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/iterator_range.hpp>

#include <core/properties.hpp>
#include <core/graph/depth.hpp>
#include <core/utils/graph_utils.hpp>

#include <classical/mig/mig_simulate.hpp>

using namespace boost::assign;

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

void mig_print_stats( const mig_graph& mig, std::ostream& os )
{
  const auto& info = mig_info( mig );
  auto n = info.inputs.size();

  std::string name = info.model_name;
  if ( name.empty() )
  {
    name = "(unnamed)";
  }

  std::vector<mig_node> outputs;
  for ( const auto& output : info.outputs )
  {
    outputs += output.first.node;
  }

  std::vector<unsigned> depths;
  const auto depth = compute_depth( mig, outputs, depths );

  os << boost::format( "[i] %20s: i/o = %7d / %7d  maj = %7d  lev = %4d" ) % name % n % info.outputs.size() % ( boost::num_vertices( mig ) - n - 1u ) % depth << std::endl;
}

std::vector<mig_function> get_children( const mig_graph& mig, const mig_node& node )
{
  std::vector<mig_function> children;

  for ( const auto& edge : boost::make_iterator_range( boost::out_edges( node, mig ) ) )
  {
    children += mig_to_function( mig, edge );
  }

  return children;
}

unsigned number_of_complemented_edges( const mig_graph& mig )
{
  const auto& complement = boost::get( boost::edge_complement, mig );

  auto count = boost::count_if( boost::make_iterator_range( edges( mig ) ), [&complement, &mig]( const mig_edge& e ) { return boost::target( e, mig ) && complement[e]; } );

  for ( const auto& output : mig_info( mig ).outputs )
  {
    if ( output.first.node && output.first.complemented ) { ++count; }
  }

  return count;
}

unsigned number_of_inverters( const mig_graph& mig )
{
  const auto inedges = precompute_ingoing_edges( mig );
  const auto& complement = boost::get( boost::edge_complement, mig );

  auto count = 0u;
  for ( const auto& p : inedges )
  {
    if ( p.first == 0 ) { continue; }
    for ( const auto& e : p.second )
    {
      if ( boost::target( e, mig ) && complement[e] )
      {
        ++count;
        break;
      }
    }
  }

  std::vector<mig_node> visited_outputs;
  for ( const auto& output : mig_info( mig ).outputs )
  {
    if ( output.first.node && output.first.complemented && boost::find( visited_outputs, output.first.node ) == visited_outputs.end() )
    {
      ++count;
      visited_outputs.push_back( output.first.node );
    }
  }

  return count;
}

std::map<mig_node, unsigned> compute_levels( const mig_graph& mig, unsigned& max_level, bool push_to_outputs )
{
  auto sa_settings = std::make_shared<properties>();
  auto sa_statistics = std::make_shared<properties>();

  auto output_levels = simulate_mig( mig, mig_depth_simulator(), sa_settings, sa_statistics );

  max_level = 0u;
  const auto& info = mig_info( mig );
  for ( const auto& o : info.outputs )
  {
    // std::cout << "[i] output " << o.second << " has level " << output_levels.at( o.first ) << std::endl;
    max_level = std::max( max_level, output_levels.at( o.first ) );
  }

  auto levels = sa_statistics->get<std::map<mig_node, unsigned>>( "node_values" );

  if ( push_to_outputs )
  {
    std::vector<mig_node> topsort( boost::num_vertices( mig ) );
    boost::topological_sort( mig, topsort.begin() );

    auto ingoing = precompute_ingoing_edges( mig );

    for ( const auto& v : topsort )
    {
      auto it = ingoing.find( v );

      /* no ingoing edges (outputs) */
      if ( it == ingoing.end() )
      {
        levels[v] = max_level;
        continue;
      }

      /* no outgoing edges (inputs) */
      if ( boost::out_degree( v, mig ) == 0u )
      {
        continue;
      }

      const auto min_edge = *boost::min_element( it->second, [&]( const mig_edge& e1, const mig_edge& e2 ) {
          return levels.at( boost::source( e1, mig ) ) < levels.at( boost::source( e2, mig ) );
        } );
      levels[v] = levels.at( boost::source( min_edge, mig ) ) - 1u;
    }
  }

  return levels;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
