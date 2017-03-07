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
