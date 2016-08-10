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
