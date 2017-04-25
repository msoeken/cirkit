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

#include "gia_utils.hpp"

#include <misc/vec/vecInt.h>

namespace abc
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

int Gia_ManMergeTopLuts( Gia_Man_t * p )
{
  int i{}, l1{}, l2{}, c = 0, s1{}, s2{};
  int * ip;
  abc::Vec_Int_t * c2, * cn;

  if ( !Gia_ManHasMapping( p ) ) return 0;
  if ( Gia_ManPoNum( p ) != 1 ) return 0;

  Gia_ManForEachLutReverse( p, i ) {
    ++c;
    if ( c == 1 ) l1 = i;
    else if ( c == 2 ) {
      l2 = i;
      break;
    }
  }

  /* fewer than 2 LUTs */
  if ( c != 2 ) return 0;

  s1 = Gia_ObjLutSize( p, l1 );
  s2 = Gia_ObjLutSize( p, l2 );

  /* copy children of l2 */
  c2 = Vec_IntAllocArrayCopy( Gia_ObjLutFanins( p, l2 ), s2 );

  /* create new children */
  cn = Vec_IntAlloc( s1 + s2 );

  Gia_LutForEachFanin( p, l1, c, i ) {
    if ( c == l2 )
    {
      Vec_IntPushArray( cn, Vec_IntArray( c2 ), Vec_IntSize( c2 ) );
    }
    else if ( Vec_IntFind( c2, c ) == -1 )
    {
      Vec_IntPush( cn, c );
    }
  }
  Vec_IntFree( c2 );

  /* override vMapping */
  c = Vec_IntGetEntry( p->vMapping, l2 );
  ip = Vec_IntGetEntryP( p->vMapping, c );
  *ip++ = Vec_IntSize( cn );
  for ( i = 0; i < Vec_IntSize( cn ); ++i )
    *ip++ = Vec_IntEntry( cn, i );
  *ip++ = l1;
  Vec_IntGrowResize( p->vMapping, c + Vec_IntSize( cn ) + 2 );
  Vec_IntSetEntry( p->vMapping, l2, 0 );
  Vec_IntSetEntry( p->vMapping, l1, c );

  Vec_IntFree( cn );

  return 1;
}

void Gia_LutTFISize_rec( Gia_Man_t * p, int index, int* ctr )
{
  int iFan, k;
  Gia_Obj_t * obj = Gia_ManObj( p, index );

  /* already visited */
  if ( obj->fMark0 || obj->fTerm ) return;

  /* increment counter */
  ( *ctr )++;
  obj->fMark0 = 1;

  Gia_LutForEachFanin( p, index, iFan, k ) {
    Gia_LutTFISize_rec( p, iFan, ctr );
  }
}

int Gia_LutTFISize( Gia_Man_t * p, int index )
{
  int ctr = 0;

  Gia_ManCleanMark0( p );

  Gia_LutTFISize_rec( p, index, &ctr );

  Gia_ManCleanMark0( p );

  return ctr;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
