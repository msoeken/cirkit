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

#include "memristor_costs.hpp"

#include <map>

#include <boost/graph/adjacency_list.hpp>
#include <boost/range/iterator_range.hpp>

#include <classical/mig/mig_utils.hpp>

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

std::pair<unsigned, unsigned> memristor_costs( const mig_graph& mig )
{
  auto max_level = 0u;
  auto levels = compute_levels( mig, max_level );

  std::map<unsigned, std::vector<mig_node>> level_to_nodes;
  for ( const auto& p : levels )
  {
    level_to_nodes[p.second].push_back( p.first );
  }

  const auto& complement = boost::get( boost::edge_complement, mig );

  auto current_max_level = 0u;
  auto current_max_size  = 0u;
  auto levels_with_complement = 0u;

  for ( auto l = 0u; l <= max_level; ++l )
  {
    auto size = 6u * level_to_nodes[l].size();
    // std::cout << "l: " << l << " " << level_to_nodes[l].size() << std::endl;
    for ( const auto& node : level_to_nodes[l] )
    {
      for ( const auto& edge : boost::make_iterator_range( boost::out_edges( node, mig ) ) )
      {
        if ( complement[edge] ) { ++size; }
      }
    }

    if ( size > current_max_size )
    {
      current_max_size = size;
      current_max_level = l;
    }

    if ( size > 6u * level_to_nodes[l].size() ) { ++levels_with_complement; }
  }

  // std::cout << "[i] current_max_size:       " << current_max_size << std::endl
  //           << "[i] levels_with_complement: " << levels_with_complement << std::endl
  //           << "[i] max_level:              " << max_level << std::endl;

  auto operations = max_level * 10u + levels_with_complement;

  return {current_max_size, operations};
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
