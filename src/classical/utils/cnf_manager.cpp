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
