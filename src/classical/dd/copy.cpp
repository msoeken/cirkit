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

#include "copy.hpp"

#include <unordered_map>

#include <core/utils/timer.hpp>
#include <classical/dd/dd_depth_first.hpp>

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

bdd bdd_copy( const bdd& f, bdd_manager& to,
              const properties::ptr& settings,
              const properties::ptr& statistics )
{
  assert( to.num_vars() >= f.manager->num_vars() );

  /* settings */
  auto shift = get( settings, "shift", to.num_vars() - f.manager->num_vars() );

  /* timing */
  properties_timer t( statistics );

  /* map "from" addresses -> "to" addresses */
  std::unordered_map<unsigned, unsigned> address_map = { {0u, 0u}, {1u, 1u} };
  for ( unsigned v = 2u; v < 2u + f.manager->num_vars(); ++v )
  {
    address_map.insert( { v, v + shift } );
  }

  auto func = [&]( const bdd& n ) {
    if ( n.index >= 2u + f.manager->num_vars() )
    {
      address_map[n.index] = to.unique_create( n.var() + shift,
                                               address_map[n.high().index],
                                               address_map[n.low().index] );
    }
  };
  dd_depth_first( f, detail::node_func_t<bdd>( func ) );

  set( statistics, "address_map", address_map );

  return bdd( &to, address_map[f.index] );
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
