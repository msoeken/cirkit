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

#include "compute_levels.hpp"

#include <vector>

#include <boost/graph/topological_sort.hpp>
#include <boost/range/algorithm.hpp>

#include <core/utils/graph_utils.hpp>
#include <core/utils/timer.hpp>
#include <classical/functions/simulate_aig.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

class aig_compute_levels_simulator : public aig_simulator<unsigned>
{
public:
  unsigned get_input( const aig_node& node, const std::string& name, unsigned pos, const aig_graph& aig ) const
  {
    return 0u;
  }

  unsigned get_constant() const
  {
    return 0u;
  }

  unsigned invert( const unsigned& v ) const
  {
    return v;
  }

  unsigned and_op( const aig_node& node, const unsigned& v1, const unsigned& v2 ) const
  {
    return std::max( v1, v2 ) + 1u;
  }
};

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

std::map<aig_node, unsigned> compute_levels( const aig_graph& aig, const properties::ptr& settings, const properties::ptr& statistics )
{
  /* settings */
  const auto push_to_outputs = get( settings, "push_to_outputs", false );

  /* timer */
  properties_timer t( statistics );

  auto sa_settings = std::make_shared<properties>();
  auto sa_statistics = std::make_shared<properties>();

  auto output_levels = simulate_aig( aig, aig_compute_levels_simulator(), sa_settings, sa_statistics );

  auto max_level = 0u;
  for ( const auto& ol : output_levels )
  {
    max_level = std::max( max_level, ol.second );
  }
  set( statistics, "max_level", max_level );

  auto levels = sa_statistics->get<std::map<aig_node, unsigned>>( "node_values" );

  if ( push_to_outputs )
  {
    std::vector<aig_node> topsort( boost::num_vertices( aig ) );
    boost::topological_sort( aig, topsort.begin() );

    auto ingoing = precompute_ingoing_edges( aig );

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
      if ( boost::out_degree( v, aig ) == 0u )
      {
        continue;
      }

      const auto min_edge = *boost::min_element( it->second, [&]( const aig_edge& e1, const aig_edge& e2 ) {
          return levels.at( boost::source( e1, aig ) ) < levels.at( boost::source( e2, aig ) );
        } );
      levels[v] = levels.at( boost::source( min_edge, aig ) ) - 1u;
    }
  }

  return levels;
}

std::vector<std::vector<aig_node>> levelize_nodes( const aig_graph& aig,
                                                   const properties::ptr& settings,
                                                   const properties::ptr& statistics )
{
  properties::ptr int_s = statistics ? statistics : std::make_shared<properties>();

  const auto l = compute_levels( aig, settings, int_s );
  const auto max_level = int_s->get<unsigned>( "max_level" );

  std::vector<std::vector<aig_node>> result( max_level + 1u, std::vector<aig_node>() );

  for ( const auto& p : l )
  {
    result[p.second].push_back( p.first );
  }

  return result;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
