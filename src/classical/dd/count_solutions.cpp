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

#include "count_solutions.hpp"

#include <map>

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

boost::multiprecision::uint256_t count_solutions( const bdd& n,
                                                  const properties::ptr& settings,
                                                  const properties::ptr& statistics )
{
  properties_timer t( statistics );

  std::map<unsigned, boost::multiprecision::uint256_t> c = { { 0u, 0 }, { 1u, 1 } };
  const boost::multiprecision::uint256_t one = 1;
  auto f = [&]( const bdd& n ) {
    c[n.index] = ( one << ( n.low().var() - n.var() - 1u ) ) * c[n.low().index] +
                 ( one << ( n.high().var() - n.var() - 1u ) ) * c[n.high().index];
  };
  dd_depth_first( n, detail::node_func_t<bdd>( f ) );

  set( statistics, "count_map", c );

  return ( one << n.var() ) * c[n.index];
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
