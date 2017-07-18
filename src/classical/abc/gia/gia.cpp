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

#include "gia.hpp"

#include <iostream>

#include <boost/format.hpp>

#include <classical/abc/functions/cirkit_to_gia.hpp>
#include <classical/abc/gia/gia_bdd.hpp>
#include <classical/abc/gia/gia_esop.hpp>
#include <classical/optimization/esop_minimization.hpp>

#include <map/if/if.h>
#include <misc/util/utilTruth.h>
#include <misc/vec/vecInt.h>

namespace abc
{
Gia_Man_t * Eso_ManCompute( Gia_Man_t * pGia, int fVerbose, Vec_Wec_t ** pvRes );
void Gia_ManLutSat( Gia_Man_t * p, int LutSize, int nNumber, int nImproves, int nBTLimit, int DelayMax, int nEdges, int fDelay, int fReverse, int fVerbose, int fVeryVerbose );
void Gia_WriteDotAigSimple( Gia_Man_t * p, char * pFileName, Vec_Int_t * vBold );
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

  Gia_ManFillValue( p );
  pNew = Gia_ManStart( nNodes + 2 );

  assert( !p->pMuxes ); /* for now */
  pNew->nConstrs = p->nConstrs;
  pNew->pName = Abc_UtilStrsav( p->pName );
  pNew->pSpec = Abc_UtilStrsav( p->pSpec );
  Gia_ManConst0( p )->Value = 0;

  for ( i = 0; i < nLeaves; ++i )
  {
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
  : p_gia( gia, &abc::Gia_ManStop )
{
}

gia_graph::gia_graph( const std::string& filename )
  : p_gia( abc::Gia_AigerRead( const_cast<char*>( filename.c_str() ), 0, 0, 0 ), &abc::Gia_ManStop )
{
}

gia_graph::gia_graph( const aig_graph& aig )
  : p_gia( cirkit_to_gia( aig ), &abc::Gia_ManStop )
{
}

gia_graph::~gia_graph()
{
  if ( p_truths )
  {
    abc::Vec_WrdFree( p_truths );
  }
}

/******************************************************************************
 * GIA properties                                                             *
 ******************************************************************************/

std::string gia_graph::input_name( int input_index ) const
{
  if ( p_gia->vNamesIn && input_index < abc::Vec_PtrSize( p_gia->vNamesIn ) )
  {
    return std::string( (char*)abc::Vec_PtrGetEntry( p_gia->vNamesIn, input_index ) );
  }
  else
  {
    return boost::str( boost::format( "input_%d" ) % input_index );
  }
}

std::string gia_graph::output_name( int output_index ) const
{
  if ( p_gia->vNamesOut && output_index < abc::Vec_PtrSize( p_gia->vNamesOut ) )
  {
    return std::string( (char*)abc::Vec_PtrGetEntry( p_gia->vNamesOut, output_index ) );
  }
  else
  {
    return boost::str( boost::format( "output_%d" ) % output_index );
  }
}

int gia_graph::num_xors() const
{
  int _num_xors = 0;
  abc::Gia_Obj_t * fan0, * fan1;

  foreach_and( [&_num_xors, &fan0, &fan1]( int index, abc::Gia_Obj_t * obj ) {
      if ( abc::Gia_ObjIsMuxType( obj ) && abc::Gia_ObjRecognizeExor( obj, &fan0, &fan1 ) )
      {
        ++_num_xors;
      }
    } );

  return _num_xors;
}

/******************************************************************************
 * Operations                                                                 *
 ******************************************************************************/

gia_graph gia_graph::cofactor( int var, bool value ) const
{
  return gia_graph( abc::Gia_ManDupCofactorVar( p_gia.get(), var, value ? 1 : 0 ) );
}

gia_graph gia_graph::select_outputs( const std::vector<int>& indexes ) const
{
  auto vec = abc::Vec_IntAllocArray( const_cast<int*>( &indexes[0] ), indexes.size() );
  auto* g = abc::Gia_ManDupSelectedOutputs( p_gia.get(), vec );
  ABC_FREE( vec ); /* do not erase the array, since it's owned by indexes */
  return gia_graph( g );
}

gia_graph gia_graph::syn3() const
{
  return gia_graph( abc::Gia_ManAigSyn3( p_gia.get(), 0, 0 ) );
}

gia_graph gia_graph::syn4() const
{
  return gia_graph( abc::Gia_ManAigSyn4( p_gia.get(), 0, 0 ) );
}

/******************************************************************************
 * Mapping                                                                    *
 ******************************************************************************/

gia_graph gia_graph::if_mapping( const properties::ptr& settings, const properties::ptr& statistics ) const
{
  /* settings */
  const auto lut_size     = get( settings, "lut_size",     6u );
  const auto area_mapping = get( settings, "area_mapping", false );
  const auto expred_cuts  = get( settings, "expred_cuts",  true );
  const auto flow_iters   = get( settings, "flow_iters",   1u );
  const auto area_iters   = get( settings, "area_iters",   2u );

  abc::If_Par_t params;

  abc::Gia_ManSetIfParsDefault( &params );
  params.nLutSize = lut_size;
  params.pLutLib  = nullptr;
  params.fArea    = area_mapping ? 1 : 0;
  params.fExpRed  = expred_cuts ? 1 : 0;
  params.nFlowIters = flow_iters;
  params.nAreaIters = area_iters;

  auto mapped_gia = abc::Gia_ManPerformMapping( p_gia.get(), &params );

  return gia_graph( mapped_gia );
}

void gia_graph::satlut_mapping( const properties::ptr& settings, const properties::ptr& statistics ) const
{
  const auto window_size    = get( settings, "window_size",    128 );
  const auto improves       = get( settings, "improves",       0 );
  const auto conflict_limit = get( settings, "conflict_limit", 100 );
  const auto verbose        = get( settings, "verbose",        false );
  const auto very_verbose   = get( settings, "very_verbose",   false );

  const auto lut_size  = max_lut_size();
  const auto delay_max = 0;
  const auto edges_max = 0;
  const auto opt_delay = false;
  const auto reverse   = false;

  abc::Gia_ManLutSat( p_gia.get(), lut_size, window_size, improves, conflict_limit, delay_max, edges_max, opt_delay, reverse, verbose, very_verbose );
}

void gia_graph::init_lut_refs() const
{
  abc::Gia_ManSetLutRefs( p_gia.get() );
}

gia_graph gia_graph::extract_lut( int index ) const
{
  return gia_graph( abc::Gia_ManDupLUT( p_gia.get(), index ) );
}

void gia_graph::init_truth_tables() const
{
  if ( !p_truths )
  {
    p_truths = abc::Vec_WrdStart( abc::Gia_ManObjNum( p_gia.get() ) );
  }
}

uint64_t gia_graph::lut_truth_table( int index ) const
{
  const auto t = abc::Gia_ObjComputeTruthTable6Lut( p_gia.get(), index, p_truths );
  return t & abc::Abc_Tt6Mask( 1 << lut_size( index ) );
}

tt gia_graph::truth_table( int output_index ) const
{
  const auto num_words = abc::Abc_Truth6WordNum( num_inputs() );
  const auto * words = abc::Gia_ObjComputeTruthTable( p_gia.get(), abc::Gia_ManCo( p_gia.get(), output_index ) );

  tt func( words, words + num_words );

  if ( num_inputs() < 6 )
  {
    tt_shrink( func, num_inputs() );
  }

  return func;
}

/******************************************************************************
 * Printing                                                                   *
 ******************************************************************************/

void gia_graph::print_stats() const
{
  abc::Gps_Par_t params{};
  abc::Gia_ManPrintStats( p_gia.get(), &params );
}

/******************************************************************************
 * I/O                                                                        *
 ******************************************************************************/

void gia_graph::write_aiger( const std::string& filename ) const
{
  abc::Gia_AigerWrite( p_gia.get(), const_cast<char*>( filename.c_str() ), 0, 0 );
}

void gia_graph::write_dot( const std::string& filename, const std::vector<int>& highlight ) const
{
  if ( highlight.empty() )
  {
    abc::Gia_WriteDotAigSimple( p_gia.get(), const_cast<char*>( filename.c_str() ), nullptr );
  }
  else
  {
    auto vec = abc::Vec_IntAllocArray( const_cast<int*>( &highlight[0] ), highlight.size() );
    abc::Gia_WriteDotAigSimple( p_gia.get(), const_cast<char*>( filename.c_str() ), vec );
    ABC_FREE( vec ); /* do not erase the array, since it's owned by highlight */
  }
}

void gia_graph::write_dot_with_luts( const std::string& filename ) const
{
  std::vector<int> highlight;
  foreach_lut( [&highlight]( int index ) { highlight.push_back( index ); } );
  write_dot( filename, highlight );
}

/******************************************************************************
 * Other logic representations                                                *
 ******************************************************************************/

gia_graph::esop_ptr gia_graph::compute_esop_cover( esop_cover_method method, const properties::ptr& settings ) const
{
  switch ( method )
  {
  case esop_cover_method::aig:
    {
      abc::Vec_Wec_t* esop = nullptr;
      abc::Eso_ManCompute( p_gia.get(), 0, &esop );
      return esop_ptr( esop, &abc::Vec_WecFree );
    } break;
  case esop_cover_method::aig_new:
    {
      return gia_extract_cover( *this, settings );
    } break;
  case esop_cover_method::aig_threshold:
    {
      if ( num_inputs() <= 10 )
      {
        abc::Vec_Wec_t* esop = nullptr;
        abc::Eso_ManCompute( p_gia.get(), 0, &esop );
        return esop_ptr( esop, &abc::Vec_WecFree );
      }
      else
      {
        return gia_extract_cover( *this, settings );
      }
    } break;
  case esop_cover_method::bdd:
    {
      Cudd mgr;

      const auto bdd = gia_to_bdd( *this, mgr, settings );

      /* get initial cover using exact PSDKRO optimization */
      exp_cache_t exp_cache;
      count_cubes_in_exact_psdkro( bdd.first.getManager(), bdd.second.front().getNode(), exp_cache );

      char * var_values = new char[bdd.first.ReadSize()];
      std::fill( var_values, var_values + bdd.first.ReadSize(), 2 );

      abc::Vec_Wec_t *esop = abc::Vec_WecAlloc( 0u );
      generate_exact_psdkro( bdd.first.getManager(), bdd.second.front().getNode(), var_values, -1, exp_cache, [&bdd, &esop, &var_values]() {
          auto * level = abc::Vec_WecPushLevel( esop );
          for ( auto i = 0; i < bdd.first.ReadSize(); ++i )
          {
            if ( var_values[i] == 2 ) continue;
            abc::Vec_IntPush( level, ( i << 1u ) | !var_values[i] );
          }
          abc::Vec_IntPush( level, -1 );
        } );

      delete[] var_values;

      return esop_ptr( esop, &abc::Vec_WecFree );
    } break;
  }

  return esop_ptr( nullptr, &abc::Vec_WecFree );
}

/******************************************************************************
 * Conversion functions                                                       *
 ******************************************************************************/

std::istream& operator>>( std::istream& in, gia_graph::esop_cover_method& cover_method )
{
  std::string token;
  in >> token;
  if ( token == "aig" || token == "0" )
  {
    cover_method = gia_graph::esop_cover_method::aig;
  }
  else if ( token == "bdd" || token == "1" )
  {
    cover_method = gia_graph::esop_cover_method::bdd;
  }
  else if ( token == "aignew" || token == "2" )
  {
    cover_method = gia_graph::esop_cover_method::aig_new;
  }
  else if ( token == "auto" || token == "3" )
  {
    cover_method = gia_graph::esop_cover_method::aig_threshold;
  }
  else
  {
    in.setstate( std::ios_base::failbit );
  }
  return in;
}

std::ostream& operator<<( std::ostream& out, const gia_graph::esop_cover_method& cover_method )
{
  switch ( cover_method )
  {
  case gia_graph::esop_cover_method::aig:
    return out << "aig";
  case gia_graph::esop_cover_method::bdd:
    return out << "bdd";
  case gia_graph::esop_cover_method::aig_new:
    return out << "aignew";
  case gia_graph::esop_cover_method::aig_threshold:
    return out << "auto";
  }

  return out;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
