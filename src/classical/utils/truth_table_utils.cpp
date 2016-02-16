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

#include "truth_table_utils.hpp"

#include <deque>
#include <iostream>

#include <boost/pending/integer_log2.hpp>

#include <core/utils/conversion_utils.hpp>

namespace cirkit
{

/******************************************************************************
 * Truth table store                                                          *
 ******************************************************************************/

tt_store& tt_store::i()
{
  static tt_store instance;
  return instance;
}

tt_store::tt_store()
{
  assert( sizeof( unsigned long ) == 8 );
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

tt tt_const0()
{
  return boost::dynamic_bitset<>( 64u, 0u );
}

tt tt_const1()
{
  return ~tt_const0();
}

tt tt_nth_var( unsigned i )
{
  if ( i < 6u )
  {
    return tt_store::i()( i );
  }
  else
  {
    tt t( 1u << i, 0ul );
    t.resize( 1u << ( i + 1 ), true );
    return t;
  }
}

unsigned tt_num_vars( const tt& t )
{
  return boost::integer_log2( t.size() );
}

void tt_extend( tt& t, unsigned to )
{
  unsigned nv = tt_num_vars( t );
  tt::size_type s, i;

  while ( nv < to )
  {
    s = t.size();
    t.resize( s << 1u );
    for ( i = 0u; i < s; ++i )
    {
      t[s + i] = t[i];
    }
    ++nv;
  }
}

void tt_shrink( tt& t, unsigned to )
{
  t.resize( 1u << to );
}

void tt_align( tt& t1, tt& t2 )
{
  unsigned nv1 = tt_num_vars( t1 ), nv2 = tt_num_vars( t2 );
  if ( nv1 < nv2 )
  {
    tt_extend( t1, nv2 );
  }
  else if ( nv2 < nv1 )
  {
    tt_extend( t2, nv1 );
  }
}

bool tt_has_var( const tt& t, unsigned i )
{
  auto n = tt_num_vars( t );

  if ( i >= n ) { return false; }

  auto tc = t;
  if ( n < tt_store::i().width ) { tt_extend( tc, tt_store::i().width ); }

  auto tv = ~tt_nth_var( i );
  tt_extend( tv, n );

  return ( (tc >> (1 << i)) & tv )  != ( tc & tv );
}

boost::dynamic_bitset<> tt_support( const tt& t )
{
  unsigned n = tt_num_vars( t );

  boost::dynamic_bitset<> support( n );
  for ( unsigned i = 0u; i < n; ++i )
  {
    support.set( i, tt_has_var( t, i ) );
  }

  return support;
}

unsigned tt_support_size( const tt& t )
{
  return tt_support( t ).count();
}

tt tt_cof0( const tt& t, unsigned i )
{
  auto n = tt_num_vars( t );
  assert( i < n );
  auto tv = ~tt_nth_var( i );
  tt_extend( tv, n );

  auto tc = t;
  if ( n < tt_store::i().width ) { tt_extend( tc, tt_store::i().width ); }

  return (tc & tv) | ((tc & tv) << (1 << i));
}

tt tt_cof1( const tt& t, unsigned i )
{
  auto n = tt_num_vars( t );
  assert( i < n );
  auto tv = tt_nth_var( i );
  tt_extend( tv, n );

  auto tc = t;
  if ( n < tt_store::i().width ) { tt_extend( tc, tt_store::i().width ); }

  return (tc & tv) | ((tc & tv) >> (1 << i));
}

bool tt_cof0_is_const0( const tt& t, unsigned i )
{
  auto n = tt_num_vars( t );
  assert( i < n );
  auto tv = ~tt_nth_var( i );
  tt_extend( tv, n );

  auto tc = t;
  if ( n < tt_store::i().width ) { tt_extend( tc, tt_store::i().width ); }

  return ( tc & tv ).empty();
}

bool tt_cof0_is_const1( const tt& t, unsigned i )
{
  auto n = tt_num_vars( t );
  assert( i < n );
  auto tv = ~tt_nth_var( i );
  tt_extend( tv, n );

  auto tc = t;
  if ( n < tt_store::i().width ) { tt_extend( tc, tt_store::i().width ); }

  return ( tc & tv ) == tv;
}

bool tt_cof1_is_const0( const tt& t, unsigned i )
{
  auto n = tt_num_vars( t );
  assert( i < n );
  auto tv = tt_nth_var( i );
  tt_extend( tv, n );

  auto tc = t;
  if ( n < tt_store::i().width ) { tt_extend( tc, tt_store::i().width ); }

  return ( tc & tv ).empty();
}

bool tt_cof1_is_const1( const tt& t, unsigned i )
{
  auto n = tt_num_vars( t );
  assert( i < n );
  auto tv = tt_nth_var( i );
  tt_extend( tv, n );

  auto tc = t;
  if ( n < tt_store::i().width ) { tt_extend( tc, tt_store::i().width ); }

  return ( tc & tv ) == tv;
}

bool tt_cofs_opposite( const tt& t, unsigned i )
{
  auto n = tt_num_vars( t );
  assert( i < n );
  auto tv = tt_nth_var( i );
  tt_extend( tv, n );

  auto tc = t;
  if ( n < tt_store::i().width ) { tt_extend( tc, tt_store::i().width ); }

  unsigned shift = ( 1u << i );
  return ((tc << shift) & tv) == (~tc & tv);
}

tt tt_exists( const tt& t, unsigned i )
{
  return tt_cof0( t, i ) | tt_cof1( t, i );
}

tt tt_forall( const tt& t, unsigned i )
{
  return tt_cof0( t, i ) & tt_cof1( t, i );
}

tt tt_permute( const tt& t, unsigned i, unsigned j )
{
  if ( i == j ) return t;

  unsigned n = tt_num_vars( t );
  assert( i < n );
  assert( j < n );

  tt vi = tt_nth_var( i );
  tt vj = tt_nth_var( j );
  tt_extend( vi, n );
  tt_extend( vj, n );

  tt c0 = tt_cof0( t, i );
  tt c1 = tt_cof1( t, i );
  tt c00 = tt_cof0( c0, j );
  tt c01 = tt_cof1( c0, j );
  tt c10 = tt_cof0( c1, j );
  tt c11 = tt_cof1( c1, j );

  return ( ~vi & ( ( ~vj & c00 ) | ( vj & c10 ) ) ) | ( vi & ( ( ~vj & c01 ) | ( vj & c11 ) ) );
}

tt tt_remove_var( const tt& t, unsigned i )
{
  unsigned n = tt_num_vars( t );
  assert( n > 0u );
  tt ret = t;

  for ( unsigned j = i; j < n - 1u; ++j )
  {
    ret = tt_permute( ret, j, j + 1u );
  }

  ret.resize( (tt::size_type)1 << ( n - 1 ) );
  return ret;
}

tt tt_flip( const tt& t, unsigned i )
{
  auto n = tt_num_vars( t );
  assert( i < n );

  auto vi = tt_nth_var( i );
  tt_extend( vi, n );

  auto tc = t;
  if ( n < tt_store::i().width ) { tt_shrink( vi, n ); }

  return ((tc << (1 << i)) & vi) | ((tc & vi) >> (1 << i));
}

void tt_to_minbase( tt& t, boost::dynamic_bitset<>* psupport )
{
  auto support = tt_support( t );

  if ( psupport ) *psupport = support;

  auto to_pos = 0u;
  auto pos = support.find_first();

  while ( pos != boost::dynamic_bitset<>::npos )
  {
    t = tt_permute( t, pos, to_pos++ );
    pos = support.find_next( pos );
  }

  /* resize */
  t.resize( 1 << support.count() );
}

void tt_to_minbase_and_discard( tt& t, unsigned max_size, boost::dynamic_bitset<>* psupport )
{
  boost::dynamic_bitset<> support = tt_support( t );

  if ( psupport ) *psupport = support;

  unsigned to_pos = 0u;
  boost::dynamic_bitset<>::size_type pos = support.find_first();

  while ( pos != boost::dynamic_bitset<>::npos )
  {
    if ( to_pos < max_size )
    {
      t = tt_permute( t, pos, to_pos++ );
    }
    else
    {
      t = tt_exists( t, pos );
    }
    pos = support.find_next( pos );
  }

  /* resize */
  t.resize( 1 << std::min( to_pos, max_size ) );
}

void tt_from_minbase( tt& t, const boost::dynamic_bitset<> pattern )
{
  std::deque<unsigned> positions;

  tt_extend( t, pattern.size() );

  boost::dynamic_bitset<>::size_type pos = pattern.find_first();
  while ( pos != boost::dynamic_bitset<>::npos )
  {
    positions.push_front( pos );
    pos = pattern.find_next( pos );
  }

  unsigned support_size = tt_support_size( t );
  assert( positions.size() == support_size );

  unsigned tpos = support_size - 1u;
  for ( const auto& pos : positions )
  {
    assert( pos >= tpos );
    t = tt_permute( t, tpos--, pos );
  }
}

std::string tt_to_hex( const tt& t )
{
  std::string s;
  to_string( t, s );

  std::string result;
  for ( unsigned i = 0u; i < s.length(); i += 4u )
  {
    result += convert_bin2hex( s.substr( i, 4u ) );
  }
  return result;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
