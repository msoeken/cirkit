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

#include "aig_to_mig.hpp"

#include <classical/functions/simulate_aig.hpp>
#include <classical/utils/aig_utils.hpp>
#include <classical/mig/mig_utils.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

class aig_to_mig_simulator : public aig_simulator<mig_function>
{
public:
  aig_to_mig_simulator( mig_graph& mig ) : mig( mig ), info( mig_info( mig ) ) {}

  mig_function get_input( const aig_node& node, const std::string& name, unsigned pos, const aig_graph& aig ) const
  {
    return { info.inputs.at( pos ), false };
  }

  mig_function get_constant() const
  {
    return mig_get_constant( mig, false );
  }

  mig_function invert( const mig_function& v ) const
  {
    return !v;
  }

  mig_function and_op( const aig_node& node, const mig_function& v1, const mig_function& v2 ) const
  {
    return mig_create_and( mig, v1, v2 );
  }

private:
  mig_graph& mig;
  const mig_graph_info& info;
};

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

mig_graph aig_to_mig( const aig_graph& aig,
                      const properties::ptr& settings,
                      const properties::ptr& statistics )
{
  mig_graph mig;
  mig_initialize( mig );

  auto& info_mig   = mig_info( mig );
  const auto& info = aig_info( aig );

  /* copy inputs */
  for ( const auto& input : info.inputs )
  {
    mig_create_pi( mig, info.node_names.at( input ) );
  }

  /* copy other info */
  info_mig.model_name = info.model_name;

  auto result = simulate_aig( aig, aig_to_mig_simulator( mig ) );

  for ( const auto& output : info.outputs )
  {
    mig_create_po( mig, result[output.first], output.second );
  }

  return mig;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
