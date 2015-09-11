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

#include "zdd_to_sets.hpp"

#include <map>

#include <boost/assign/std/vector.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>

using namespace boost::assign;

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

using set_t = boost::dynamic_bitset<>;
using set_set_t = std::vector<set_t>;
using visited_t = std::map<unsigned, set_set_t>;

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

set_set_t zdd_to_sets_rec( const zdd& z, visited_t& visited )
{
  using boost::adaptors::transformed;

  /* visited before? */
  auto it = visited.find( z.index );
  if ( it != visited.end() )
  {
    return it->second;
  }

  /* recur */
  auto high = zdd_to_sets_rec( z.high(), visited );
  auto low  = zdd_to_sets_rec( z.low(),  visited );

  boost::for_each( high, [&z]( set_t& s ) { s.set( z.var() ); } );

  set_set_t r;
  boost::push_back( r, high );
  boost::push_back( r, low );
  return r;
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

set_set_t zdd_to_sets( const zdd& z )
{
  visited_t visited;
  set_set_t s0;
  visited.insert( {0u, s0 } );
  set_set_t s1; set_t e( z.manager->num_vars() ); s1 += e;
  visited.insert( {1u, s1} );

  return zdd_to_sets_rec( z, visited );
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
