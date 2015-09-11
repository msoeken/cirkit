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

#include "aig_from_cirkit_bdd.hpp"

#include <functional>
#include <map>
#include <vector>

#include <boost/format.hpp>
#include <boost/range/algorithm.hpp>

#include <core/utils/timer.hpp>
#include <classical/utils/aig_utils.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

aig_function aig_from_bdd_rec( aig_graph& aig, const bdd& node,
                               std::map<unsigned, aig_function>& visited,
                               bool complement_optimization )
{
  auto it = visited.find( node.index );
  if ( it != visited.end() )
  {
    return it->second;
  }

  if ( complement_optimization )
  {
    auto comp = !node;
    it = visited.find( comp.index );
    if ( it != visited.end() )
    {
      return {it->second.node, !it->second.complemented};
    }
  }

  const auto& info = aig_info( aig );

  auto index = node.var();

  auto f_true  = aig_from_bdd_rec( aig, node.high(), visited, complement_optimization );
  auto f_false = aig_from_bdd_rec( aig, node.low(), visited, complement_optimization );

  auto func = aig_create_ite( aig, {info.inputs[index], false}, f_true, f_false );
  visited.insert( {node.index, func} );

  return func;
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

std::vector<aig_function> aig_from_bdd( aig_graph& aig, const std::vector<bdd>& fs,
                                        const properties::ptr& settings, const properties::ptr& statistics )
{
  using namespace std::placeholders;

  assert( !fs.empty() );

  /* settings */
  auto input_labels            = get( settings, "input_labels", std::vector<std::string>() );
  auto complement_optimization = get( settings, "complement_optimization", false );

  /* run-time */
  properties_timer t( statistics );

  const auto& info = aig_info( aig );

  auto n = fs.front().manager->num_vars();
  auto num_pis = info.inputs.size();

  for ( auto i = num_pis; i < n; ++i )
  {
    aig_create_pi( aig, i < input_labels.size() ? input_labels.at( i ) : boost::str( boost::format( "x%d" ) % i ) );
  }

  std::map<unsigned, aig_function> visited = { {0u, aig_get_constant( aig, false )}, {1u, aig_get_constant( aig, true )} };

  std::vector<aig_function> result( fs.size() );
  boost::transform( fs, result.begin(), std::bind( aig_from_bdd_rec, std::ref( aig ), _1, std::ref( visited ), complement_optimization ) );
  return result;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
