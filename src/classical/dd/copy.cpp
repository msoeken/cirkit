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
