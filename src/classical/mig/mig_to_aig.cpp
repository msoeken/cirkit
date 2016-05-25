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

#include "mig_to_aig.hpp"

#include <classical/mig/mig_simulate.hpp>
#include <classical/utils/aig_utils.hpp>
#include <classical/mig/mig_utils.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

class mig_to_aig_simulator : public mig_simulator<aig_function>
{
public:
  mig_to_aig_simulator( aig_graph& aig ) : aig( aig ), info( aig_info( aig ) ) {}

  aig_function get_input( const mig_node& node, const std::string& name, unsigned pos, const mig_graph& mig ) const
  {
    return { info.inputs.at( pos ), false };
  }

  aig_function get_constant() const
  {
    return aig_get_constant( aig, false );
  }

  aig_function invert( const aig_function& v ) const
  {
    return !v;
  }

  aig_function maj_op( const mig_node& node, const aig_function& v1, const aig_function& v2, const aig_function& v3 ) const
  {
    return aig_create_maj( aig, v1, v2, v3 );
  }

private:
  aig_graph& aig;
  const aig_graph_info& info;
};

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

aig_graph mig_to_aig( const mig_graph& mig,
                      const properties::ptr& settings,
                      const properties::ptr& statistics )
{
  aig_graph aig;
  aig_initialize( aig );

  auto& info_aig   = aig_info( aig );
  const auto& info = mig_info( mig );

  /* copy inputs */
  for ( const auto& input : info.inputs )
  {
    aig_create_pi( aig, info.node_names.at( input ) );
  }

  /* copy other info */
  info_aig.model_name = info.model_name;

  auto result = simulate_mig( mig, mig_to_aig_simulator( aig ) );

  for ( const auto& output : info.outputs )
  {
    aig_create_po( aig, result[output.first], output.second );
  }

  return aig;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
