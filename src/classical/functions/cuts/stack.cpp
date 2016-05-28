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

#include "stack.hpp"

#include <vector>

#include <boost/dynamic_bitset.hpp>
#include <boost/graph/topological_sort.hpp>

#include <core/utils/bitset_utils.hpp>
#include <core/utils/timer.hpp>
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

std::map<mig_node, structural_cut> stack_based_structural_cut_enumeration( const mig_graph& circ, unsigned k,
                                                                           const properties::ptr& settings,
                                                                           const properties::ptr& statistics )
{
  /* settings */
  const auto include_constant = get( settings, "include_constant", false );

  /* timer */
  properties_timer t( statistics );

  const auto n = num_vertices( circ );
  std::map<mig_node, structural_cut> cut_map;

  std::vector<mig_node> topsort( n );
  boost::topological_sort( circ, topsort.begin() );

  for ( auto node : topsort )
  {
    if ( node == 0u && !include_constant )
    {
      cut_map.insert( {0u, {boost::dynamic_bitset<>( n )}} );
    }
    else if ( out_degree( node, circ ) == 0u )
    {
      cut_map.insert( {node, {onehot_bitset( n, node )}} );
    }
    else
    {
      structural_cut cuts;
      const auto children = get_children( circ, node );
      for ( const auto& v1 : cut_map[children[0u].node] )
      {
        for ( const auto& v2 : cut_map[children[1u].node] )
        {
          auto cut = v1 | v2;
          if ( cut.count() > k ) { continue; }

          for ( const auto& v3 : cut_map[children[2u].node] )
          {
            cut |= v3;
            if ( cut.count() > k ) { continue; }
            cuts += cut;
          }
        }
      }

      cuts += onehot_bitset( n, node );
      cut_map.insert( {node, cuts} );
    }
  }

  return cut_map;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
