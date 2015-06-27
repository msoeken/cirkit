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

/**
 * @file fanout_free_regions.hpp
 *
 * @brief Computs fanout free regions from a general graph
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef FANOUT_FREE_REGIONS_HPP
#define FANOUT_FREE_REGIONS_HPP

#include <deque>
#include <map>
#include <vector>

#include <boost/assign/std/vector.hpp>
#include <boost/range/iterator_range.hpp>

#include <core/utils/graph_utils.hpp>
#include <core/utils/range_utils.hpp>
#include <core/utils/timer.hpp>

using namespace boost::assign;

namespace cirkit
{

namespace detail
{

template<typename Graph>
void compute_ffr_inputs_rec( const vertex_t<Graph>& v,
                             const vertex_t<Graph>& ffr_output,
                             std::deque<vertex_t<Graph>>& ffr_outputs,
                             std::vector<vertex_t<Graph>>& ffr_inputs,
                             const std::vector<unsigned>& indegrees,
                             const Graph& g )
{
  /* primary input? */
  if ( boost::out_degree( v, g ) == 0u )
  {
    ffr_inputs += v;
  }
  /* if ffr output? */
  else if ( v != ffr_output && indegrees.at( v ) > 1u )
  {
    ffr_inputs += v;
    ffr_outputs.push_back( v );
  }
  else
  {
    for ( const auto& adj : boost::make_iterator_range( boost::adjacent_vertices( v, g ) ) )
    {
      compute_ffr_inputs_rec( adj, ffr_output, ffr_outputs, ffr_inputs, indegrees, g );
    }
  }
}

template<typename Graph>
std::vector<vertex_t<Graph>> compute_ffr_inputs( const vertex_t<Graph>& output,
                                                 std::deque<vertex_t<Graph>>& ffr_outputs,
                                                 const std::vector<unsigned>& indegrees,
                                                 const Graph& g )
{
  std::vector<vertex_t<Graph>> ffr_inputs;
  compute_ffr_inputs_rec( output, output, ffr_outputs, ffr_inputs, indegrees, g );
  return ffr_inputs;
}

}

/**
 * @brief Computs fanout free regions
 *
 * The graph must be acyclic and directed from the outputs to the inputs.
 *
 * The key in the returned map is an output of a fanout free region, i.e.,
 * either a primary output or a vertex with more than one ingoing edge.
 *
 * The vertices in the value of the returned map is an input of a fanout free
 * region, i.e., either a primary input or a vertex with more than one ingoing
 * edge.
 *
 * If the outputs are known in advance, then they can be provided to the algorithm
 * via the outputs setting.  Otherwise, it requires an O(|V|) loop to determine
 * them.
 */
template<typename Graph>
std::map<vertex_t<Graph>, std::vector<vertex_t<Graph>>> fanout_free_regions( const Graph& g,
                                                                             const properties::ptr& settings = properties::ptr(),
                                                                             const properties::ptr& statistics = properties::ptr() )
{
  std::map<vertex_t<Graph>, std::vector<vertex_t<Graph>>> result;

  /* settings */
  const auto verbose = get( settings, "verbose", false );
        auto outputs = get( settings, "outputs", std::vector<vertex_t<Graph>>() );

  /* run-time */
  properties_timer t( statistics );

  /* pre-compute indegrees */
  auto indegrees = precompute_in_degrees( g );

  /* compute outputs if not provided */
  if ( outputs.empty() )
  {
    for ( const auto& v : boost::make_iterator_range( boost::vertices( g ) ) )
    {
      if ( !indegrees[v] )
      {
        outputs += v;
      }
    }
  }

  /* dequeue for keeping track of FFR outputs */
  std::deque<vertex_t<Graph>> ffr_outputs( outputs.begin(), outputs.end() );

  /* compute each FFR */
  while ( !ffr_outputs.empty() )
  {
    auto ffr_output = ffr_outputs.front();
    if ( result.find( ffr_output ) == result.end() )
    {
      auto ffr_inputs = detail::compute_ffr_inputs( ffr_output, ffr_outputs, indegrees, g );

      if ( verbose )
      {
        std::cout << boost::format( "[i] found ffr region %d(%s)" ) % ffr_output % any_join( ffr_inputs, ", " ) << std::endl;
      }

      result.insert( {ffr_output, ffr_inputs} );
    }
    ffr_outputs.pop_front();
  }

  return result;
}

template<typename Graph>
std::vector<vertex_t<Graph>> topological_sort_ffrs( const std::map<vertex_t<Graph>, std::vector<vertex_t<Graph>>>& ffrs,
                                                    const std::vector<vertex_t<Graph>>& topsort )
{
  std::vector<vertex_t<Graph>> ffrs_topsort;

  for ( const auto& v : topsort )
  {
    if ( v != 0u && ffrs.find( v ) != ffrs.end() )
    {
      ffrs_topsort += v;
    }
  }

  return std::move( ffrs_topsort );
}

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
