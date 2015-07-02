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

#include <map>
#include <queue>
#include <vector>

#include <boost/assign/std/vector.hpp>
#include <boost/bimap.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/range/algorithm.hpp>
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
struct topsort_compare_t
{
  explicit topsort_compare_t( const std::vector<vertex_t<Graph>>& topsortinv ) : topsortinv( topsortinv ) {}

  bool operator()( const vertex_t<Graph>& v1, const vertex_t<Graph>& v2 ) const
  {
    return topsortinv.at( v1 ) < topsortinv.at( v2 );
  }

private:
  const std::vector<vertex_t<Graph>>& topsortinv;
};

template<typename Graph>
using ffr_output_queue_t = std::priority_queue<vertex_t<Graph>, std::vector<vertex_t<Graph>>, detail::topsort_compare_t<Graph>>;

template<typename Graph>
using relabel_queue_t = std::priority_queue<vertex_t<Graph>>;

template<typename Graph>
void compute_ffr_inputs_rec( const vertex_t<Graph>& v,
                             const vertex_t<Graph>& ffr_output,
                             ffr_output_queue_t<Graph>& ffr_outputs,
                             std::vector<vertex_t<Graph>>& ffr_inputs,
                             const std::vector<unsigned>& indegrees,
                             bool relabel, relabel_queue_t<Graph>& relabel_queue,
                             const Graph& g )
{
  /* relabel? */
  if ( relabel )
  {
    relabel_queue.push( v );
  }

  /* primary input? */
  if ( boost::out_degree( v, g ) == 0u )
  {
    if ( boost::find( ffr_inputs, v ) == ffr_inputs.end() )
    {
      ffr_inputs += v;
    }
  }
  /* if ffr output? */
  else if ( v != ffr_output && indegrees.at( v ) > 1u )
  {
    ffr_inputs += v;
    ffr_outputs.push( v );
  }
  else
  {
    for ( const auto& adj : boost::make_iterator_range( boost::adjacent_vertices( v, g ) ) )
    {
      compute_ffr_inputs_rec( adj, ffr_output, ffr_outputs, ffr_inputs, indegrees, relabel, relabel_queue, g );
    }
  }
}

template<typename Graph>
std::vector<vertex_t<Graph>> compute_ffr_inputs( const vertex_t<Graph>& output,
                                                 ffr_output_queue_t<Graph>& ffr_outputs,
                                                 const std::vector<unsigned>& indegrees,
                                                 bool relabel, relabel_queue_t<Graph>& relabel_queue,
                                                 const Graph& g )
{
  std::vector<vertex_t<Graph>> ffr_inputs;
  compute_ffr_inputs_rec( output, output, ffr_outputs, ffr_inputs, indegrees, relabel, relabel_queue, g );
  return ffr_inputs;
}

}

template<typename Graph>
using relabel_map_t = std::map<vertex_t<Graph>, boost::bimap<unsigned, vertex_t<Graph>>>;

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
  const auto verbose      = get( settings, "verbose",      false );
        auto outputs      = get( settings, "outputs",      std::vector<vertex_t<Graph>>() );
  const auto relabel      = get( settings, "relabel",      false );
  const auto has_constant = get( settings, "has_constant", false );

  /* run-time */
  properties_timer t( statistics );

  /* relabeling */
  relabel_map_t<Graph> relabel_map;

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

  /* topological sort */
  std::vector<vertex_t<Graph>> topsort( boost::num_vertices( g ) ), topsortinv( boost::num_vertices( g ) );
  boost::topological_sort( g, topsort.begin() );
  for ( const auto& v : index( topsort ) ) { topsortinv[v.value] = v.index; }

  /* dequeue for keeping track of FFR outputs */
  detail::topsort_compare_t<Graph> comp( topsortinv );
  detail::ffr_output_queue_t<Graph> ffr_outputs( outputs.begin(), outputs.end(), comp );

  /* compute each FFR */
  while ( !ffr_outputs.empty() )
  {
    auto ffr_output = ffr_outputs.top();
    if ( result.find( ffr_output ) == result.end() )
    {
      detail::relabel_queue_t<Graph> relabel_queue;

      if ( relabel && has_constant )
      {
        relabel_queue.push( 0u );
      }

      auto ffr_inputs = detail::compute_ffr_inputs( ffr_output, ffr_outputs, indegrees, relabel, relabel_queue, g );

      if ( verbose )
      {
        std::cout << boost::format( "[i] found ffr region %d(%s)" ) % ffr_output % any_join( ffr_inputs, ", " ) << std::endl;
      }

      result.insert( {ffr_output, ffr_inputs} );

      if ( relabel )
      {
        typename relabel_map_t<Graph>::mapped_type bm;

        auto pos  = 0u;
        auto last = -1;
        while ( !relabel_queue.empty() )
        {
          auto val = relabel_queue.top();

          if ( (int)val != last )
          {
            bm.insert( typename relabel_map_t<Graph>::mapped_type::value_type( pos++, val ) );
            last = val;
          }

          relabel_queue.pop();
        }

        relabel_map.insert( {ffr_output, bm} );
      }
    }
    ffr_outputs.pop();
  }

  if ( relabel )
  {
    set( statistics, "relabel_map", relabel_map );
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
