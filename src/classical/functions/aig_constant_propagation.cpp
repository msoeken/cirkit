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

#include "aig_constant_propagation.hpp"

#include <boost/assign/std/vector.hpp>

#include <core/utils/timer.hpp>
#include <classical/utils/aig_utils.hpp>
#include <classical/utils/simulate_aig.hpp>

using namespace boost::assign;

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

using node_set_t     = std::set<aig_node>;
using edge_set_t     = std::set<aig_edge>;
using filter_graph_t = boost::filtered_graph<aig_graph, boost::is_not_in_subset<edge_set_t>, boost::is_not_in_subset<node_set_t>>;

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

class aig_constant_propagation_simulator : public aig_simulator<aig_function>
{
public:
  aig_constant_propagation_simulator( aig_graph& aig_new, const std::map<std::string, bool>& values )
    : aig_new( aig_new ),
      values( values )
  {
  }

  aig_function get_input( const aig_node& node, const std::string& name, unsigned pos, const aig_graph& aig ) const
  {
    const auto it = values.find( name );
    if ( it == values.end() )
    {
      return aig_create_pi( aig_new, name );
    }
    else
    {
      return aig_get_constant( aig_new, it->second );
    }
  }

  aig_function get_constant() const
  {
    return aig_get_constant( aig_new, false );
  }

  aig_function invert( const aig_function& v ) const
  {
    return !v;
  }

  aig_function and_op( const aig_node& node, const aig_function& v1, const aig_function& v2 ) const
  {
    return aig_create_and( aig_new, v1, v2 );
  }

private:
  aig_graph& aig_new;
  const std::map<std::string, bool>& values;
};

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

aig_graph aig_constant_propagation( const aig_graph& aig, const std::map<std::string, bool>& values,
                                    const properties::ptr& settings, const properties::ptr& statistics )
{
  /* settings */

  /* run-time */
  properties_timer t( statistics );

  aig_graph aig_new;
  aig_initialize( aig_new );

  auto& info_new   = aig_info( aig_new );
  const auto& info = aig_info( aig );

  /* copy other info */
  info_new.model_name = info.model_name;

  auto result = simulate_aig( aig, aig_constant_propagation_simulator( aig_new, values ) );

  for ( const auto& output : info.outputs )
  {
    aig_create_po( aig_new, result[output.first], output.second );
  }

  return aig_new;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
