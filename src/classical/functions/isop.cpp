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

#include "isop.hpp"

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

tt tt_isop( const tt& on, const tt& ondc, std::vector<int>& cover )
{
  assert( on.size() == ondc.size() );
  assert( ( on & ~ondc ).none() );

  /* terminal cases */
  if ( on.none() ) { return on; }
  if ( ondc.all() )
  {
    cover.push_back( 0 );
    return ondc;
  }

  const auto num_vars = tt_num_vars( on );
  int var = num_vars - 1;
  for ( ; var >= 0; --var )
  {
    if ( tt_has_var( on, var ) || tt_has_var( ondc, var ) )
    {
      break;
    }
  }
  assert( var >= 0 );

  auto on0   = tt_cof0( on, var );   tt_shrink( on0, num_vars );
  auto on1   = tt_cof1( on, var );   tt_shrink( on1, num_vars );
  auto ondc0 = tt_cof0( ondc, var ); tt_shrink( ondc0, num_vars );
  auto ondc1 = tt_cof1( ondc, var ); tt_shrink( ondc1, num_vars );

  auto beg0 = cover.size();
  auto res0 = tt_isop( on0 & ~ondc1, ondc0, cover );
  auto end0 = cover.size();
  auto res1 = tt_isop( on1 & ~ondc0, ondc1, cover );
  auto end1 = cover.size();
  auto res2 = tt_isop( ( on0 & ~res0 ) | ( on1 & ~res1 ), ondc0 & ondc1, cover );

  auto tv = tt_nth_var( var );
  if ( num_vars < 6u )
  {
    tt_shrink( tv, num_vars );
  }
  else if ( num_vars > 6u )
  {
    tt_align( res0, tv );
  }
  res2 |= ( res0 & ~tv ) | ( res1 & tv );

  for ( auto c = beg0; c < end0; ++c )
  {
    cover[c] |= 1u << ( var << 1u );
  }
  for ( auto c = end0; c < end1; ++c )
  {
    cover[c] |= 1u << ( ( var << 1u ) + 1 );
  }

  assert( ( on & ~res2 ).none() );
  assert( ( res2 & ~ondc ).none() );

  return res2;
}

std::vector<int> tt_cnf( const tt& f )
{
  std::vector<int> cover;
  const auto n = tt_num_vars( f );

  tt_isop( f, f, cover );
  auto cs = cover.size();
  for ( auto c = 0u; c < cs; ++c )
  {
    cover[c] |= 1u << ( n << 1u );
  }
  tt_isop( ~f, ~f, cover );
  for ( auto c = cs; c < cover.size(); ++c )
  {
    cover[c] |= 1u << ( ( n << 1u ) + 1u );
  }

  return cover;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
