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
