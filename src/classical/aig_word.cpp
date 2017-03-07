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

#include "aig_word.hpp"

#include <boost/assign/std/vector.hpp>
#include <boost/format.hpp>

namespace cirkit
{

using namespace boost::assign;

aig_word to_aig_word( const aig_function& f )
{
  return aig_word(1u, f);
}

const aig_function &to_aig_function( const aig_word& w )
{
  assert( w.size() == 1 );
  return w[0];
}

aig_word aig_create_wi( aig_graph& aig, const unsigned width, const std::string& name )
{
  assert( width > 0 && "Width must not be < 0" );
  aig_word w;
  for ( unsigned u = 0u; u < width; ++u )
  {
    const std::string n = (boost::format("%s[%d]") % name % u).str();
    w += ( aig_create_pi( aig, n ) );
  }
  return w;
}

void aig_create_wo( aig_graph& aig, const aig_word& w, const std::string& name )
{
  unsigned index = 0u;
  for ( auto& e : w )
  {
    const std::string n = (boost::format("%s[%d]") % name % index++).str();
    aig_create_po( aig, e, n);
  }
}

aig_word aig_create_bvbin( aig_graph& aig, const std::string& value )
{
  assert( value.size() > 0u );
  aig_word w( value.size() );
  for ( std::size_t s = 0u; s < value.size(); ++s )
  {
    assert( value[s] == '0' || value[s] == '1' );
    w[ value.size() - 1u - s] = aig_get_constant( aig, value[s] == '1' );
  }
  return w;
}

aig_function aig_create_equal( aig_graph& aig, const aig_word& left, const aig_word& right )
{
  assert( left.size() == right.size() );
  aig_function result = aig_get_constant( aig, false );
  for ( unsigned u = 0u; u < left.size(); ++u )
  {
    const aig_function current = aig_create_xor( aig, left[u], right[u] );
    result = aig_create_or( aig, current, result );
  }
  return !result;
}

aig_function aig_create_bvsgt( aig_graph& aig, const aig_word& left, const aig_word& right )
{
  assert( left.size() == right.size() );
  assert( left.size() > 0u );

  auto li = left.rbegin(), ri = right.rbegin(), end = left.rend();

  aig_function not_l = !*li;
  aig_function not_r = !*ri;
  aig_function result = aig_create_and( aig, not_l, *ri );
  aig_function equal = !aig_create_xor( aig, *li, *ri );

  for ( ++li, ++ri; li != end; ++li, ++ri )
  {
    not_r = !*ri;

    const auto now_great = aig_create_and( aig, *li, not_r );
    const auto now = aig_create_and( aig, equal, now_great );

    const auto now_equal = !aig_create_xor( aig, *li, *ri );
    equal = aig_create_and( aig, now_equal, equal );

    result = aig_create_or( aig, result, now );
  }

  return result;
}

aig_function aig_create_bvslt( aig_graph& aig, const aig_word& left, const aig_word& right )
{
  assert( left.size() == right.size() );
  assert( left.size() > 0u );

  auto li = left.rbegin(), ri = right.rbegin(), end = left.rend();

  aig_function not_l = !*li;
  aig_function result = aig_create_and( aig, *li, not_l );
  aig_function equal = !aig_create_xor( aig, *li, *ri );

  for ( ++li, ++ri; li != end; ++li, ++ri )
  {
    not_l = !*li;

    const auto now_less = aig_create_and( aig, not_l, *ri );
    const auto now = aig_create_and( aig, equal, now_less );

    const auto now_equal = !aig_create_xor( aig, *li, *ri );
    equal = aig_create_and( aig, now_equal, equal );

    result = aig_create_or( aig, result, now );
  }

  return result;
}

aig_function aig_create_bvsge( aig_graph& aig, const aig_word& left, const aig_word& right )
{
  assert( left.size() == right.size() );
  assert( left.size() > 0u );

  auto li = left.rbegin(), ri = right.rbegin(), end = left.rend();

  aig_function not_l = !*li;
  aig_function not_r = !*ri;
  aig_function great = aig_create_and( aig, not_l, *ri );
  aig_function equal = !aig_create_xor( aig, *li, *ri );
  aig_function result = great;

  for ( ++li, ++ri; li != end; ++li, ++ri )
  {
    not_r = !*ri;
    const auto now_great = aig_create_and( aig, *li, not_r );
    const auto now = aig_create_and( aig, equal, now_great );

    const auto now_equal = !aig_create_xor( aig, *li, *ri );
    equal = aig_create_and( aig, now_equal, equal );

    result = aig_create_or( aig, result, now );
  }
  return aig_create_or( aig, result, equal );
}

aig_function aig_create_bvsle( aig_graph& aig, const aig_word& left, const aig_word& right )
{
  assert( left.size() == right.size() );
  assert( left.size() > 0u );

  auto li = left.rbegin(), ri = right.rbegin(), end = left.rend();

  aig_function not_r = !*ri;
  aig_function less = aig_create_and( aig, *li, not_r );
  aig_function equal = !aig_create_xor( aig, *li, *ri );
  aig_function result = less;

  for ( ++li, ++ri; li != end; ++li, ++ri )
  {
    const aig_function not_l = !*li;
    const aig_function now_less = aig_create_and( aig, not_l, *ri );
    const aig_function now = aig_create_and( aig, equal, now_less );

    const aig_function now_equal = !aig_create_xor( aig, *li, *ri );
    equal = aig_create_and( aig, now_equal, equal );

    result = aig_create_or( aig, result, now );
  }
  return aig_create_or( aig, result, equal );
}

aig_function aig_create_bvule( aig_graph& aig, const aig_word& left, const aig_word& right )
{
  assert( left.size() == right.size() );
  assert( left.size() > 0 );

  auto li = left.rbegin(), ri = right.rbegin(), end = left.rend();
  aig_function not_l = !*li;
  aig_function less = aig_create_and( aig, not_l, *ri );
  aig_function equal = !aig_create_xor( aig, *li, *ri );
  aig_function ret = less;

  for (++li, ++ri; li != end; ++li, ++ri) {
    not_l = !*li;
    aig_function now_less = aig_create_and(aig, not_l, *ri);
    aig_function now = aig_create_and(aig, equal, now_less);
    aig_function now_equal = !aig_create_xor(aig, *li, *ri);
    equal = aig_create_and(aig, now_equal, equal);
    ret = aig_create_or(aig, ret, now);
  }
  ret = aig_create_or(aig, ret, equal);
  return ret;
}

aig_function aig_create_bvult( aig_graph& aig, const aig_word& left, const aig_word& right )
{
  assert( left.size() == right.size() );
  assert( left.size() > 0 );

  auto li = left.rbegin(), ri = right.rbegin(), end = left.rend();
  aig_function not_l = !*li;
  aig_function ret = aig_create_and(aig, not_l, *ri );
  aig_function equal = !aig_create_xor( aig, *li, *ri );

  for (++li, ++ri; li != end; ++li, ++ri) {
    not_l = !*li;
    aig_function now_less = aig_create_and(aig, not_l, *ri);
    aig_function now = aig_create_and(aig, equal, now_less);
    aig_function now_equal = !aig_create_xor(aig, *li, *ri);
    equal = aig_create_and(aig, now_equal, equal);
    ret = aig_create_or(aig, ret, now);
  }
  return ret;
}

aig_function aig_create_bvugt( aig_graph& aig, const aig_word& left, const aig_word& right )
{
  assert( left.size() == right.size() );
  assert( left.size() > 0 );

  auto li = left.rbegin(), ri = right.rbegin(), end = left.rend();
  aig_function not_r = !*ri;
  aig_function ret = aig_create_and(aig, *li, not_r );
  aig_function equal = !aig_create_xor( aig, *li, *ri );

  for (++li, ++ri; li != end; ++li, ++ri) {
    not_r = !*ri;
    aig_function now_great = aig_create_and(aig, *li, not_r);
    aig_function now = aig_create_and(aig, equal, now_great);
    aig_function now_equal = !aig_create_xor(aig, *li, *ri);
    equal = aig_create_and(aig, now_equal, equal);
    ret = aig_create_or(aig, ret, now);
  }
  return ret;
}

aig_function aig_create_bvuge( aig_graph& aig, const aig_word& left, const aig_word& right )
{
  assert( left.size() == right.size() );
  assert( left.size() > 0 );

  auto li = left.rbegin(), ri = right.rbegin(), end = left.rend();
  aig_function not_r = !*ri;
  aig_function great = aig_create_and(aig, *li, not_r );
  aig_function equal = !aig_create_xor( aig, *li, *ri );
  aig_function ret = great;

  for (++li, ++ri; li != end; ++li, ++ri) {
    not_r = !*ri;
    aig_function now_great = aig_create_and(aig, *li, not_r);
    aig_function now = aig_create_and(aig, equal, now_great);
    aig_function now_equal = !aig_create_xor(aig, *li, *ri);
    equal = aig_create_and(aig, now_equal, equal);
    ret = aig_create_or(aig, ret, now);
  }
  ret = aig_create_or(aig, ret, equal);
  return ret;
}

aig_word aig_create_bvand( aig_graph& aig, const aig_word& left, const aig_word& right )
{
  assert( left.size() == right.size() );
  aig_word result( left.size() );
  for ( unsigned u = 0; u < left.size(); ++u ) {
    result[u] = aig_create_and( aig, left[u], right[u] );
  }
  return result;
}

aig_word aig_create_bvor( aig_graph& aig, const aig_word& left, const aig_word& right )
{
  assert( left.size() == right.size() );
  aig_word result( left.size() );
  for ( unsigned u = 0; u < left.size(); ++u ) {
    result[u] = aig_create_or( aig, left[u], right[u] );
  }
  return result;
}

aig_word aig_create_bvxor( aig_graph& aig, const aig_word& left, const aig_word& right )
{
  assert( left.size() == right.size() );
  aig_word result( left.size() );
  for ( unsigned u = 0; u < left.size(); ++u ) {
    result[u] = aig_create_xor( aig, left[u], right[u] );
  }
  return result;
}

aig_word aig_create_bvadd( aig_graph& aig, const aig_word& left, const aig_word& right )
{
  assert( left.size() == right.size() );

  aig_word result( left.size() );
  aig_function carry = aig_get_constant( aig, false );

  aig_function xor1, or1, and1, and2;
  for ( unsigned u = 0u; u < left.size(); ++u )
  {
    xor1 = aig_create_xor( aig, left[u], right[u] );
    result[u] = aig_create_xor( aig, xor1, carry );

    // left & right | carry & ( left | right )
    and1 = aig_create_and( aig, left[u], right[u] );
    or1 = aig_create_or( aig, left[u], right[u] );
    and2 = aig_create_and( aig, carry, or1 );
    carry = aig_create_or( aig, and1, and2 );
  }

  return result;
}

aig_word shift_left( aig_graph& aig, const aig_word& w, unsigned value )
{
  aig_word result( w.size() );
  if ( value == 0u )
  {
    return w;
  }
  for ( unsigned i = 0u; i < w.size(); ++i )
  {
    if ( i < value )
    {
      result[ i ] = aig_get_constant( aig, false );
    }
    else
    {
      result[ i ] = w[ i - value ];
    }
  }
  return result;
}

aig_word aig_create_bvmul( aig_graph& aig, const aig_word& left, const aig_word& right )
{
  assert( left.size() == right.size() );

  aig_word result( left.size(), aig_get_constant( aig, false ) );

  aig_word tmp;
  for ( unsigned i = 0; i < result.size(); ++i )
  {
    tmp = aig_create_sext( aig, left.size() - 1u, to_aig_word( left[i] ) );
    tmp = aig_create_bvand( aig, right, tmp );
    tmp = shift_left( aig, tmp, i );

    result = aig_create_bvadd( aig, result, tmp );
  }
  return result;
}

aig_word aig_create_bvnot( aig_graph& aig, const aig_word& w )
{
  aig_word result( w.size() );
  for ( unsigned u = 0u; u < w.size(); ++u )
  {
    result[ u ] = !w[ u ];
  }
  return result;
}

aig_word aig_create_bvneg( aig_graph& aig, const aig_word& w )
{
  aig_word result( w.size(), aig_get_constant( aig, false ) );
  result.front() = aig_get_constant( aig, true );
  const aig_word tmp = aig_create_bvnot( aig, w );
  return aig_create_bvadd( aig, tmp, result );
}

aig_word aig_create_bvsub( aig_graph& aig, const aig_word& left, const aig_word& right )
{
  aig_word result( aig_create_bvneg( aig, right ) );
  return aig_create_bvadd( aig, left, result );
}

aig_word aig_create_sext( aig_graph& aig, const unsigned width, const aig_word& w )
{
  assert( w.size() > 0u );
  aig_word result( w.size() + width, w.back() );
  std::copy( w.begin(), w.end(), result.begin() );
  return result;
}

aig_word aig_create_zext( aig_graph& aig, const unsigned width, const aig_word& w )
{
  assert( w.size() > 0u );
  aig_word result( w.size() + width, aig_get_constant( aig, false ) );
  std::copy( w.begin(), w.end(), result.begin() );
  return result;
}

aig_word aig_create_ite( aig_graph& aig, const aig_function& cond, const aig_word& t, const aig_word& e )
{
  assert( t.size() == e.size() );
  const std::size_t size = t.size();
  aig_word result( size,  aig_get_constant( aig, false ) );
  for ( unsigned u = 0u; u < size; ++u )
  {
    result[ u ] = aig_create_ite( aig, cond, t[ u ], e[ u ] );
  }
  return result;
}

std::ostream& operator<<( std::ostream& os, const aig_word &w )
{
  os << "[ ";
  for ( const auto& it : w )
  {
    os << it << " ";
  }
  os << "]";
  return os;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
