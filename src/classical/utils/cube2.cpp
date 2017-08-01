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

#include "cube2.hpp"

namespace cirkit
{


/******************************************************************************
 * Constructors                                                               *
 ******************************************************************************/

cube2::cube2() : value( 0 ) {}

cube2::cube2( uint32_t bits, uint32_t mask )
  : bits( bits ), mask( mask )
{
}

/******************************************************************************
 * Query operations (unary)                                                   *
 ******************************************************************************/

int cube2::num_literals() const
{
  return __builtin_popcount( mask );
}

/******************************************************************************
 * Query operations (binary)                                                  *
 ******************************************************************************/

int cube2::distance( const cube2& that ) const
{
  return __builtin_popcount( ( bits ^ that.bits ) | ( mask ^ that.mask ) );
}

uint32_t cube2::differences( const cube2& that ) const
{
  return ( bits ^ that.bits ) | ( mask ^ that.mask );
}

bool cube2::operator==( const cube2& that ) const
{
  return value == that.value;
}

bool cube2::operator!=( const cube2& that ) const
{
  return value != that.value;
}

/******************************************************************************
 * Operators (binary)                                                         *
 ******************************************************************************/

cube2 cube2::operator&( const cube2& that ) const
{
  /* literals must agree on intersection */
  const auto int_mask = mask & that.mask;
  if ( ( bits ^ that.bits ) & int_mask )
  {
    return zero_cube();
  }

  return cube2( bits | that.bits, mask | that.mask );
}

/* it is assumed that this and that have distance 1 */
cube2 cube2::merge( const cube2& that ) const
{
  const auto d = ( bits ^ that.bits ) | ( mask ^ that.mask );
  return cube2( bits ^ ( ~that.bits & d ), mask ^ ( that.mask & d ) );
}

std::array<cube2, 4> cube2::exorlink( const cube2& that, int distance, uint32_t differences, unsigned* group ) const
{
  uint32_t tbits, tmask;
  uint32_t tpos;

  std::array<cube2, 4> res;

  const auto cbits = ~bits & ~that.bits;
  const auto cmask = mask ^ that.mask;

  for ( int i = 0; i < distance; ++i )
  {
    tbits = bits; /* start from this */
    tmask = mask; /* start from this */
    tpos  = differences;

    for ( int j = 0; j < distance; ++j )
    {
      /* compute next position */
      const uint64_t p = tpos & -tpos;
      tpos &= tpos - 1;

      switch ( *group++ )
      {
      case 0:
        /* take from this */
        break;
      case 1:
        /* take from that */
        tbits ^= ( ( that.bits & p ) ^ tbits ) & p;
        tmask ^= ( ( that.mask & p ) ^ tmask ) & p;
        break;
      case 2:
        /* take other */
        tbits ^= ( ( cbits & p ) ^ tbits ) & p;
        tmask ^= ( ( cmask & p ) ^ tmask ) & p;
        break;
      }
    }

    res[i].bits = tbits;
    res[i].mask = tmask;
  }

  return res;
}

/******************************************************************************
 * Modify operations                                                          *
 ******************************************************************************/

/* invert all literals */
void cube2::invert_all()
{
  bits ^= mask;
}

void cube2::rotate( unsigned bit )
{
  const auto bits_tmp = bits;
  bits ^= ( ~bits ^ mask ) & ( 1 << bit );
  mask ^= ~bits_tmp & ( 1 << bit );
}

/******************************************************************************
 * Construction                                                               *
 ******************************************************************************/

cube2 cube2::one_cube()
{
  return cube2( 0, 0 );
}

cube2 cube2::zero_cube()
{
  return cube2( ~0, 0 );
}

cube2 cube2::elementary_cube( unsigned index )
{
  const auto bits = 1 << index;
  return cube2( bits, bits );
}

/******************************************************************************
 * Pringing / debugging                                                       *
 ******************************************************************************/

void cube2::print( unsigned length, std::ostream& os ) const
{
  for ( auto i = 0u; i < length; ++i )
  {
    os << ( ( ( mask >> i ) & 1 ) ? ( ( ( bits >> i ) & 1 ) ? '1' : '0' ) : '-' );
  }
}

std::ostream& operator<<( std::ostream& os, const cube2& cube )
{
  cube.print( 32u, os );
  return os;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
