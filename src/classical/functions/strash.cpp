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

strash_simulator::strash_simulator( aig_graph& aig_new, unsigned offset,
                                    const std::map<unsigned, unsigned>& reorder,
                                    const boost::dynamic_bitset<>& invert )
  : aig_new( aig_new ),
    info( aig_info( aig_new ) ),
    offset( offset ),
    reorder( reorder ),
    _invert( invert ) {}

aig_function strash_simulator::get_input( const aig_node& node, const std::string& name, unsigned pos, const aig_graph& aig ) const
{
  const auto it = reorder.find( pos );
  return { info.inputs.at( offset + ( it == reorder.end() ? pos : it->second ) ), _invert[pos] };
}

aig_function strash_simulator::get_constant() const
{
  return aig_get_constant( aig_new, false );
}

aig_function strash_simulator::invert( const aig_function& v ) const
{
  return !v;
}

aig_function strash_simulator::and_op( const aig_node& node, const aig_function& v1, const aig_function& v2 ) const
{
  return aig_create_and( aig_new, v1, v2 );
}

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
  const auto& info = aig_info( aig );

  /* settings */
  const auto reorder      = get( settings, "reorder",      std::map<unsigned, unsigned>() );
  const auto invert       = get( settings, "invert",       boost::dynamic_bitset<>( info.inputs.size() ) );
  const auto reuse_inputs = get( settings, "reuse_inputs", false );

  auto& info_dest  = aig_info( aig_dest );
  const auto offset = !reuse_inputs ? info_dest.inputs.size() : 0u;

  /* copy unateness and symmetries */
  if ( offset == 0u && reorder.empty() && invert.none() )
  {
    info_dest.unateness = info.unateness;
    info_dest.input_symmetries = info.input_symmetries;
  }

  /* copy inputs */
  if ( !reuse_inputs )
  {
    for ( const auto& input : info.inputs )
    {
      aig_create_pi( aig_dest, info.node_names.at( input ) );
    }
  }

  /* copy other info */
  info_dest.model_name = info.model_name;

  auto result = simulate_aig( aig, strash_simulator( aig_dest, offset, reorder, invert ) );

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
