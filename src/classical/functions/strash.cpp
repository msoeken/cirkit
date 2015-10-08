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

#include "strash.hpp"

#include <classical/functions/simulate_aig.hpp>
#include <classical/utils/aig_utils.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

class strash_simulator : public aig_simulator<aig_function>
{
public:
  strash_simulator( aig_graph& aig_new, const std::map<unsigned, unsigned>& reorder )
    : aig_new( aig_new ),
      info( aig_info( aig_new ) ),
      reorder( reorder ) {}

  aig_function get_input( const aig_node& node, const std::string& name, unsigned pos, const aig_graph& aig ) const
  {
    const auto it = reorder.find( pos );
    return { info.inputs.at( it == reorder.end() ? pos : it->second ), false };
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
  const aig_graph_info& info;
  const std::map<unsigned, unsigned>& reorder;
};

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

aig_graph strash( const aig_graph& aig,
                  const properties::ptr& settings,
                  const properties::ptr& statistics )
{
  aig_graph aig_new;
  aig_initialize( aig_new );

  strash( aig, aig_new, settings, statistics );

  return aig_new;
}

void strash( const aig_graph& aig,
             aig_graph& aig_dest,
             const properties::ptr& settings,
             const properties::ptr& statistics )
{
  /* settings */
  const auto reorder = get( settings, "reorder", std::map<unsigned, unsigned>() );

  auto& info_dest  = aig_info( aig_dest );
  const auto& info = aig_info( aig );

  /* copy inputs */
  for ( const auto& input : info.inputs )
  {
    aig_create_pi( aig_dest, info.node_names.at( input ) );
  }

  /* copy other info */
  info_dest.model_name = info.model_name;

  auto result = simulate_aig( aig, strash_simulator( aig_dest, reorder ) );

  for ( const auto& output : info.outputs )
  {
    aig_create_po( aig_dest, result[output.first], output.second );
  }
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
