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
  if ( ( ~ondc ).none() )
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
  tt_cnf( f, cover );
  return cover;
}

void tt_cnf( const tt& f, std::vector<int>& cover )
{
  const auto n = tt_num_vars( f );

  auto cs = cover.size();
  tt_isop( f, f, cover );
  for ( auto c = cs; c < cover.size(); ++c )
  {
    cover[c] |= 1u << ( n << 1u );
  }
  cs = cover.size();
  tt_isop( ~f, ~f, cover );
  for ( auto c = cs; c < cover.size(); ++c )
  {
    cover[c] |= 1u << ( ( n << 1u ) + 1u );
  }
}

cube_vec_t cover_to_cubes( const std::vector<int>& cover, unsigned num_vars )
{
  cube_vec_t sop;

  for ( auto c : cover )
  {
    const auto cube_bv = boost::dynamic_bitset<>( num_vars << 1, c );
    cube term( num_vars );
    for ( auto i = 0u; i < num_vars; ++i )
    {
      assert( !( cube_bv[i << 1] && cube_bv[( i << 1 ) + 1] ) );
      if ( cube_bv[i << 1] )
      {
        term[i] = '0';
      }
      else if ( cube_bv[(i << 1) + 1] )
      {
        term[i] = '1';
      }
      /* else don't care by default */
    }
    sop.push_back( term );
  }

  return sop;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
