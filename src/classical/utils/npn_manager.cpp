/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2017  EPFL
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
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

npn_manager::npn_manager( unsigned hash_table_size, const npn_classifier_t& npn_func )
  : table( hash_table_size ),
    npn_func( npn_func )
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

    if ( entry.tt == to_string( tt ) )
    {
      ++cache_hit;
      npn = boost::dynamic_bitset<>( entry.npn );
      perm = std::vector<unsigned>( entry.perm );
      phase = boost::dynamic_bitset<>( entry.phase );
    }
    else
    {
      ++cache_miss;
      increment_timer t( &runtime );
      npn = npn_func( tt, phase, perm );

      entry.tt    = to_string( tt );
      entry.npn   = to_string( npn );
      entry.perm  = perm;
      entry.phase = phase;
    }
  }
  else
  {
    increment_timer t( &runtime );
    npn = npn_func( tt, phase, perm );
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
