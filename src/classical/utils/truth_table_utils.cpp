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

#include "truth_table_utils.hpp"

#include <deque>
#include <iostream>

#include <boost/pending/integer_log2.hpp>

#include <core/utils/conversion_utils.hpp>
#include <core/utils/string_utils.hpp>

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

void tt_resize( tt& t, unsigned size )
{
  auto num_vars = tt_num_vars( t );
  if ( num_vars == size ) return;
  else if ( num_vars < size )
  {
    tt_extend( t, size );
  }
  else
  {
    tt_shrink( t, size );
  }
}

bool tt_is_const0( const tt& t )
{
  tt t0 = tt_const0();
  tt_resize( t0, tt_num_vars( t ) );
  return t0 == t;
}

bool tt_is_const1( const tt& t )
{
  tt t1 = tt_const1();
  tt_resize( t1, tt_num_vars( t ) );
  return t1 == t;
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

  auto tt_new = ( ~vi & ( ( ~vj & c00 ) | ( vj & c10 ) ) ) | ( vi & ( ( ~vj & c01 ) | ( vj & c11 ) ) );

  if ( n < 6u )
  {
    tt_shrink( tt_new, n );
  }

  return tt_new;
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

tt tt_from_hex( const std::string& s )
{
  const auto bin = convert_hex2bin( s );
  auto t = tt( bin.size(), 0u );
  for ( auto i = 0u; i < bin.size(); ++i )
  {
    assert( bin[i] == '0' || bin[i] == '1' );
    t[ t.size() - i - 1 ] = ( bin[i] == '1' );
  }
  return t;
}

tt tt_from_hex( const std::string& s, unsigned to )
{
  auto t = tt_from_hex( s );
  const auto num_vars = tt_num_vars( t );
  if ( num_vars < to )
  {
    tt_extend( t, to );
  }
  else if ( num_vars > to )
  {
    tt_shrink( t, to );
  }
  return t;
}

/******************************************************************************
 * truth table from expression                                                *
 ******************************************************************************/

std::pair<tt, unsigned> tt_expression_evaluator::on_const( bool value ) const
{
  return {value ? tt_const1() : tt_const0(), 0u};
}

std::pair<tt, unsigned> tt_expression_evaluator::on_var( unsigned index ) const
{
  return {tt_nth_var( index ), index + 1u};
}

std::pair<tt, unsigned> tt_expression_evaluator::on_inv( const std::pair<tt, unsigned>& value ) const
{
  return {~( value.first ), value.second};
}

std::pair<tt, unsigned> tt_expression_evaluator::on_and( const std::pair<tt, unsigned>& value1, const std::pair<tt, unsigned>& value2 ) const
{
  auto _v1 = value1.first;
  auto _v2 = value2.first;
  tt_align( _v1, _v2 );
  return {_v1 & _v2, std::max( value1.second, value2.second )};
}

std::pair<tt, unsigned> tt_expression_evaluator::on_or( const std::pair<tt, unsigned>& value1, const std::pair<tt, unsigned>& value2 ) const
{
  auto _v1 = value1.first;
  auto _v2 = value2.first;
  tt_align( _v1, _v2 );
  return {_v1 | _v2, std::max( value1.second, value2.second )};
}

std::pair<tt, unsigned> tt_expression_evaluator::on_maj( const std::pair<tt, unsigned>& value1, const std::pair<tt, unsigned>& value2, const std::pair<tt, unsigned>& value3 ) const
{
  auto _v1 = value1.first;
  auto _v2 = value2.first;
  auto _v3 = value3.first;
  tt_align( _v1, _v2 );
  tt_align( _v2, _v3 );
  tt_align( _v1, _v3 );
  return {( _v1 & _v2 ) | ( _v1 & _v3 ) | ( _v2 & _v3 ), std::max( value1.second, std::max( value2.second, value3.second ) )};
}

std::pair<tt, unsigned> tt_expression_evaluator::on_xor( const std::pair<tt, unsigned>& value1, const std::pair<tt, unsigned>& value2 ) const
{
  auto _v1 = value1.first;
  auto _v2 = value2.first;
  tt_align( _v1, _v2 );
  return {_v1 ^ _v2, std::max( value1.second, value2.second )};
}

tt tt_from_expression( const expression_t::ptr& expr )
{
  const auto v = evaluate_expression( expr, tt_expression_evaluator() );
  auto t = v.first;
  tt_shrink( t, v.second );
  return t;
}

tt tt_from_sop_spec( const std::string& spec )
{
  enum class pla_type_t { none, on, off };
  auto pla_type = pla_type_t::none;
  tt f( 1u );
  
  foreach_string( spec, "\n", [&f, &pla_type]( const std::string& line ) {
      const auto pair = split_string_pair( line, " " );
      const auto p = pair.first;

      switch ( pla_type )
      {
      case pla_type_t::none:
        pla_type = ( pair.second == "1" ) ? pla_type_t::on : pla_type_t::off;
        f = tt( 1u << p.size() );
        if ( pla_type == pla_type_t::off )
        {
          f.flip();
        }
        break;
      case pla_type_t::on:   assert( pair.second == "1" ); break;
      case pla_type_t::off:  assert( pair.second == "0" ); break;
      }

      auto cube = ( pla_type == pla_type_t::on ) ? ~tt( 1 << p.size() ) : tt( 1 << p.size() );
      for ( auto i = 0u; i < p.size(); ++i )
      {
        if ( p[i] == '-' ) continue;
        auto v = ( p[i] == '0' ) != ( pla_type == pla_type_t::off ) ? ~tt_nth_var( i ) : tt_nth_var( i );
        if ( p.size() < 6 )
        {
          tt_shrink( v, p.size() );
        }
        else
        {
          tt_align( v, cube );
        }
        
        if ( pla_type == pla_type_t::on )
        {
          cube &= v;
        }
        else
        {
          cube |= v;
        }
      }

      if ( pla_type == pla_type_t::on )
      {
        f |= cube;
      }
      else
      {
        f &= cube;
      }
    } );

  return f;
}

std::vector<int> walsh_spectrum( const tt& func )
{
  const auto n = tt_num_vars( func );

  std::vector<int> spectra( func.size(), 0u );
  foreach_bit( func, [&spectra]( unsigned pos ) { spectra[pos] = 1u; } );

  /* butterfly loops */
  for ( auto i = 0u; i < n; ++i )
  {
    auto i1 = 0u;

    const unsigned d = ( 1 << ( n - 1 - i ) );
    /* blocks? */
    for ( auto b = 0u; b < ( 1u << i ); ++b, i1 += d )
    {
      /* block elements */
      for ( auto e = 0u; e < d; ++e, ++i1 )
      {
        const auto i2 = i1 + d;

        const auto v1 = spectra[i1] + spectra[i2];
        const auto v2 = spectra[i1] - spectra[i2];
        spectra[i1] = v1;
        spectra[i2] = v2;
      }
    }
  }

  return spectra;
}

tt tt_maj(tt a, tt b, tt c)
{
  auto max_num_vars = std::max( tt_num_vars( a ), std::max( tt_num_vars( b ), tt_num_vars( c ) ) );

  tt_extend( a, max_num_vars );
  tt_extend( b, max_num_vars );
  tt_extend( c, max_num_vars );

  return (a & b) | (b & c) | (a & c);
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
