/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2016  EPFL
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

#include "gia.hpp"

#include <iostream>

#include <classical/abc/functions/cirkit_to_gia.hpp>

#include <map/if/if.h>
#include <misc/vec/vecInt.h>

namespace abc
{
Gia_Man_t * Eso_ManCompute( Gia_Man_t * pGia, int fVerbose, Vec_Wec_t ** pvRes );
}

/******************************************************************************
 * Missing ABC methods                                                        *
 ******************************************************************************/

namespace abc
{

void Gia_ManGetCone_rec( Gia_Man_t * p, Gia_Obj_t * pObj, Vec_Int_t * vNodes )
{
  if ( Vec_IntFind( vNodes, Gia_ObjId( p, pObj ) ) >= 0 ) return;

  Gia_ManGetCone_rec( p, Gia_ObjFanin0( pObj ), vNodes );
  Gia_ManGetCone_rec( p, Gia_ObjFanin1( pObj ), vNodes );
  if ( Gia_ObjIsMux( p, pObj ) )
  {
    Gia_ManGetCone_rec( p, Gia_ObjFanin2( p, pObj ), vNodes );
  }
  Vec_IntPush( vNodes, Gia_ObjId( p, pObj ) );
}

void Gia_ManGetCone( Gia_Man_t * p, Gia_Obj_t * pObj, int * pLeaves, int nLeaves, Vec_Int_t * vNodes )
{
  int i;

  Vec_IntClear( vNodes );

  for ( i = 0; i < nLeaves; ++i )
  {
    Vec_IntPush( vNodes, pLeaves[i] );
  }
  Gia_ManGetCone_rec( p, pObj, vNodes );
}

Gia_Man_t * Gia_ManDupLUT( Gia_Man_t* p, int index )
{
  int i, j = 0, nLeaves, nNodes;
  Gia_Man_t * pNew;
  Gia_Obj_t * pObj;
  Vec_Int_t * vNodes;

  vNodes = Vec_IntAlloc( 20 );
  nLeaves = Gia_ObjLutSize( p, index );
  Gia_ManGetCone( p, Gia_ManObj( p, index ), Gia_ObjLutFanins( p, index ), nLeaves, vNodes );
  nNodes = Vec_IntSize( vNodes );

  Vec_IntPrint( vNodes );

  Gia_ManFillValue( p );
  pNew = Gia_ManStart( nNodes + 2 );

  std::cout << "nLeaves = " << nLeaves << " nNodes = " << nNodes << " nObjs " << pNew->nObjs << std::endl;

  assert( !p->pMuxes ); /* for now */
  pNew->nConstrs = p->nConstrs;
  pNew->pName = Abc_UtilStrsav( p->pName );
  pNew->pSpec = Abc_UtilStrsav( p->pSpec );
  Gia_ManConst0( p )->Value = 0;

  for ( i = 0; i < nLeaves; ++i )
  {
    std::cout << "i = " << i << std::endl;
    Gia_ManObj( p, Vec_IntEntry( vNodes, i ) )->Value = Gia_ManAppendCi( pNew );
  }

  for ( i = nLeaves; i < nNodes; ++i )
  {
    pObj = Gia_ManObj( p, Vec_IntEntry( vNodes, i ) );
    j = pObj->Value = Gia_ManAppendAnd( pNew, Gia_ObjFanin0Copy( pObj ), Gia_ObjFanin1Copy( pObj ) );
  }

  /* add PO */
  Gia_ManAppendCo( pNew, j );

  Vec_IntFree( vNodes );

  return pNew;
}

}

namespace cirkit
{

/******************************************************************************
 * (De)Constructors                                                           *
 ******************************************************************************/

gia_graph::gia_graph( abc::Gia_Man_t* gia )
  : p_gia( gia )
{
}

gia_graph::gia_graph( const std::string& filename )
  : p_gia( abc::Gia_AigerRead( const_cast<char*>( filename.c_str() ), 0, 0, 0 ) )
{
}

gia_graph::gia_graph( const aig_graph& aig )
  : p_gia( cirkit_to_gia( aig ) )
{
}

gia_graph::~gia_graph()
{
  if ( p_gia )
  {
    abc::Gia_ManStop( p_gia );
    p_gia = nullptr;
  }
}

/******************************************************************************
 * Mapping                                                                    *
 ******************************************************************************/

gia_graph gia_graph::if_mapping( const properties::ptr& settings, const properties::ptr& statistics )
{
  /* settings */
  const auto lut_size     = get( settings, "lut_size",     6u );
  const auto area_mapping = get( settings, "area_mapping", false );

  abc::If_Par_t params;

  abc::Gia_ManSetIfParsDefault( &params );
  params.nLutSize = lut_size;
  params.pLutLib  = nullptr;
  params.fArea    = area_mapping ? 1 : 0;

  auto mapped_gia = abc::Gia_ManPerformMapping( p_gia, &params );

  return gia_graph( mapped_gia );
}

gia_graph gia_graph::extract_lut( int index ) const
{
  return gia_graph( abc::Gia_ManDupLUT( p_gia, index ) );
}

/******************************************************************************
 * Printing                                                                   *
 ******************************************************************************/

void gia_graph::print_stats() const
{
  abc::Gps_Par_t params{};
  abc::Gia_ManPrintStats( p_gia, &params );
}

/******************************************************************************
 * I/O                                                                        *
 ******************************************************************************/

void gia_graph::write_aiger( const std::string& filename ) const
{
  abc::Gia_AigerWrite( p_gia, const_cast<char*>( filename.c_str() ), 0, 0 );
}

/******************************************************************************
 * Other logic representations                                                *
 ******************************************************************************/

gia_graph::esop_ptr gia_graph::compute_esop_cover() const
{
  abc::Vec_Wec_t* esop = nullptr;
  abc::Eso_ManCompute( p_gia, 0, &esop );
  return esop_ptr( esop, &abc::Vec_WecFree );
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
