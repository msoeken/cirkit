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

#include "npn_manager.hpp"

#include <boost/format.hpp>

#include <core/utils/timer.hpp>
#include <classical/functions/npn_canonization.hpp>

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

npn_manager::npn_manager( unsigned num_vars, unsigned hash_table_size )
  : num_vars( num_vars ),
    table( hash_table_size )
{
}

tt npn_manager::compute( const tt& tt, boost::dynamic_bitset<>& phase, std::vector<unsigned>& perm )
{
  boost::dynamic_bitset<> npn;

  /* compute NPN and use hash table if possible */
  if ( !table.empty() )
  {
    const auto ttu      = tt.to_ulong();
    const auto hash_key = ttu % table.size();
    auto& entry         = table[hash_key];

    if ( entry.tt == ttu )
    {
      ++cache_hit;
      npn = boost::dynamic_bitset<>( 1u << num_vars, entry.npn );
      perm = std::vector<unsigned>( entry.perm );
      phase = boost::dynamic_bitset<>( entry.phase );
    }
    else
    {
      ++cache_miss;
      increment_timer t( &runtime );
      npn = exact_npn_canonization( tt, phase, perm );

      entry.tt    = ttu;
      entry.npn   = npn.to_ulong();
      entry.perm  = perm;
      entry.phase = phase;
    }
  }
  else
  {
    increment_timer t( &runtime );
    npn = exact_npn_canonization( tt, phase, perm );
  }

  return npn;
}

void npn_manager::print_statistics( std::ostream& os ) const
{
  os << boost::format( "[i] NPN manager: size = %d   cache hits = %d   cache misses = %d   run-time = %.2f secs" ) % table.size() % cache_hit % cache_miss % runtime << std::endl;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
