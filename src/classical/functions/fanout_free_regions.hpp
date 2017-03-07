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

/******************************************************************************
 * Topologically sorted output list                                           *
 ******************************************************************************/

template<typename Graph>
struct topsort_compare_t
{
  explicit topsort_compare_t( const Graph& g )
    : topsortinv( num_vertices( g ) )
  {
    std::vector<vertex_t<Graph>> topsort( num_vertices( g ) );
    boost::topological_sort( g, topsort.begin() );
    for ( const auto& v : index( topsort ) ) { topsortinv[v.value] = v.index; }
  }

  bool operator()( const vertex_t<Graph>& v1, const vertex_t<Graph>& v2 ) const
  {
    return topsortinv.at( v1 ) < topsortinv.at( v2 );
  }

private:
  std::vector<vertex_t<Graph>> topsortinv;
};

template<typename Graph>
using ffr_output_queue_t = std::priority_queue<vertex_t<Graph>, std::vector<vertex_t<Graph>>, topsort_compare_t<Graph>>;

template<typename Graph>
ffr_output_queue_t<Graph> make_output_queue( std::vector<vertex_t<Graph>>& outputs, const std::vector<unsigned>& indegrees, const Graph& g )
{
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
  topsort_compare_t<Graph> comp( g );
  return ffr_output_queue_t<Graph>( outputs.begin(), outputs.end(), comp );
}

/******************************************************************************
 * Relabelling queue                                                          *
 ******************************************************************************/

template<typename Graph>
using relabel_queue_t = std::priority_queue<vertex_t<Graph>>;

template<typename Graph>
using relabel_map_t = std::map<vertex_t<Graph>, boost::bimap<unsigned, vertex_t<Graph>>>;



namespace detail
{

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

template<typename Graph>
std::vector<vertex_t<Graph>> compute_ffr_inputs_bfs( const vertex_t<Graph>& output,
                                                     const std::vector<unsigned>& indegrees,
                                                     unsigned max_inputs,
                                                     const Graph& g )
{
  std::queue<vertex_t<Graph>> q;
  q.push( output );

  std::vector<vertex_t<Graph>> ffr_inputs;

  while ( !q.empty() && ( q.size() + ffr_inputs.size() ) < max_inputs )
  {
    const auto top = q.front(); q.pop();

    if ( out_degree( top, g ) == 0u || ( top != output && indegrees.at( top ) > 1u ) )
    {
      if ( boost::find( ffr_inputs, top ) == ffr_inputs.end() )
      {
        ffr_inputs += top;
      }
    }
    else
    {
      for ( const auto& adj : boost::make_iterator_range( adjacent_vertices( top, g ) ) )
      {
        q.push( adj );
      }
    }
  }

  while ( !q.empty() )
  {
    if ( boost::find( ffr_inputs, q.front() ) == ffr_inputs.end() )
    {
      ffr_inputs += q.front();
    }
    q.pop();
  }

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
  const auto verbose      = get( settings, "verbose",      false );
        auto outputs      = get( settings, "outputs",      std::vector<vertex_t<Graph>>() );
  const auto relabel      = get( settings, "relabel",      false );
  const auto has_constant = get( settings, "has_constant", false );
  //const auto max_inputs   = get( settings, "max_inputs",   std::numeric_limits<unsigned>::max() );

  /* run-time */
  properties_timer t( statistics );

  /* relabeling */
  relabel_map_t<Graph> relabel_map;

  /* pre-compute indegrees */
  auto indegrees = precompute_in_degrees( g );

  /* output queue */
  auto ffr_outputs = make_output_queue( outputs, indegrees, g );

  /* compute each FFR */
  while ( !ffr_outputs.empty() )
  {
    auto ffr_output = ffr_outputs.top();
    if ( result.find( ffr_output ) == result.end() )
    {
      relabel_queue_t<Graph> relabel_queue;

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
std::map<vertex_t<Graph>, std::vector<vertex_t<Graph>>> fanout_free_regions_bfs( const Graph& g,
                                                                                 const properties::ptr& settings = properties::ptr(),
                                                                                 const properties::ptr& statistics = properties::ptr() )
{
  std::map<vertex_t<Graph>, std::vector<vertex_t<Graph>>> result;

  /* settings */
  const auto verbose      = get( settings, "verbose",      false );
        auto outputs      = get( settings, "outputs",      std::vector<vertex_t<Graph>>() );
  const auto max_inputs   = get( settings, "max_inputs",   std::numeric_limits<unsigned>::max() );

  /* run-time */
  properties_timer t( statistics );

  /* pre-compute indegrees */
  const auto indegrees = precompute_in_degrees( g );

  /* output queue */
  auto ffr_outputs = make_output_queue( outputs, indegrees, g );

  /* compute each FFR */
  while ( !ffr_outputs.empty() )
  {
    const auto ffr_output = ffr_outputs.top(); ffr_outputs.pop();
    if ( result.find( ffr_output ) != result.end() ) { continue; }

    const auto ffr_inputs = detail::compute_ffr_inputs_bfs( ffr_output, indegrees, max_inputs, g );

    for ( const auto& input : ffr_inputs )
    {
      if ( out_degree( input, g ) > 0u )
      {
        ffr_outputs.push( input );
      }
    }

    if ( verbose )
    {
      std::cout << boost::format( "[i] found ffr region %d(%s)" ) % ffr_output % any_join( ffr_inputs, ", " ) << std::endl;
    }

    result.insert( {ffr_output, ffr_inputs} );
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
