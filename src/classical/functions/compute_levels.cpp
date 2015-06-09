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

class compute_levels_simulator : public aig_simulator<unsigned>
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

  auto output_levels = simulate_aig( aig, compute_levels_simulator(), sa_settings, sa_statistics );

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

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
