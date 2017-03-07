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

#include "arithmetic.hpp"

#include <boost/range/counting_range.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

/* move? */
bdd ite( const bdd& cond, const bdd& _then, const bdd& _else )
{
  return ( !cond && _else ) || ( cond && _then );
}

std::tuple<bdd, bdd> full_adder( const bdd& x, const bdd& y, const bdd& cin )
{
  const auto sum  = x ^ y ^ cin;
  const auto cout = ( x && y ) || ( x && cin ) || ( y && cin );

  return std::make_tuple( sum, cout );
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

std::vector<bdd> bdd_subtract( const std::vector<bdd>& minuend, const std::vector<bdd>& subtrahend )
{
  assert( minuend.size() == subtrahend.size() );
  assert( !minuend.empty() );

  std::vector<bdd> diff( minuend.size() );
  bdd carry = minuend.front().manager->bdd_top();

  for ( const auto& i : boost::counting_range( 0ul, static_cast<unsigned long>( minuend.size() ) ) )
  {
    std::tie( diff[i], carry ) = full_adder( minuend[i], !subtrahend[i], carry );
  }

  return diff;
}

std::vector<bdd> bdd_abs( const std::vector<bdd>& n )
{
  std::vector<bdd> mask( n.size(), n.back() );
  std::vector<bdd> result;

  for ( const auto& i : boost::counting_range( 0ul, static_cast<unsigned long>( n.size() ) ) )
  {
    result.push_back( n[i] ^ mask[i] );
  }

  auto diff = bdd_subtract( result, mask );
  std::copy( diff.begin(), diff.begin() + n.size(), result.begin() );

  return result;
}

std::vector<bdd> zero_extend( const std::vector<bdd>& n, unsigned to )
{
  assert( to >= n.size() );
  auto ze = n;
  ze.resize( to, n.front().manager->bdd_bot() );
  return ze;
}

std::vector<bdd> sign_extend( const std::vector<bdd>& n, unsigned to )
{
  assert( to >= n.size() );
  auto se = n;
  se.resize( to, n.back() );
  return se;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
