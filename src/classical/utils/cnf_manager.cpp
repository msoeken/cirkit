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

#include "cnf_manager.hpp"

#include <boost/format.hpp>

#include <core/utils/bitset_utils.hpp>
#include <core/utils/timer.hpp>
#include <classical/functions/isop.hpp>

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

cnf_manager::vertex_range_t cnf_manager::compute( const tt& func, unsigned* literal_count )
{
  const auto key = to_string( func );
  const auto it = hash.find( key );

  unsigned begin = 0u, end = 0u, count = 0u;

  if ( it != hash.end() )
  {
    ++cache_hit;
    begin = std::get<0>( it->second );
    end   = std::get<1>( it->second );
    count = std::get<2>( it->second );
  }
  else
  {
    ++cache_miss;
    increment_timer t( &runtime );
    begin = covers.size();
    tt_cnf( func, covers );
    end = covers.size();

    /* count literals */
    const auto num_vars = tt_num_vars( func );
    for ( auto i = begin; i < end; ++i )
    {
      for ( auto x = 0u; x < num_vars; ++x )
      {
        if ( ( covers[i] >> ( x << 1 ) ) & 3 )
        {
          ++count;
        }
      }
    }

    hash[key] = std::make_tuple( begin, end, count );
  }

  if ( literal_count )
  {
    *literal_count = count;
  }

  return boost::make_iterator_range( covers.begin() + begin, covers.begin() + end );
}

void cnf_manager::print_statistics( std::ostream& os ) const
{
  os << boost::format( "[i] CNF manager: size = %d   cache hits = %d   cache misses = %d   run-time = %.2f secs" ) % hash.size() % cache_hit % cache_miss % runtime << std::endl;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
