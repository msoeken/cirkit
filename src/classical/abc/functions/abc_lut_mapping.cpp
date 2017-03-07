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

#include "abc_lut_mapping.hpp"

#include <unordered_map>

#include <core/utils/timer.hpp>
#include <classical/utils/truth_table_utils.hpp>
#include <classical/abc/abc_api.hpp>
#include <classical/abc/functions/cirkit_to_gia.hpp>

#include <map/if/if.h>

namespace abc
{
Hop_Obj_t * Abc_ObjHopFromGia( Hop_Man_t * pHopMan, Gia_Man_t * p, int GiaId, Vec_Ptr_t * vCopies );
}

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

abc::Gia_Man_t* aig_mapping( const aig_graph& aig, unsigned k )
{
  auto* abc_aig = cirkit_to_gia( aig );
  abc::If_Par_t Pars, *pPars = &Pars;
  abc::Gia_ManSetIfParsDefault( pPars );
  pPars->nLutSize = k;
  auto * abc_lut = abc::Gia_ManPerformMapping( abc_aig, pPars );

  abc::Gia_ManStop( abc_aig );

  return abc_lut;
}

void map_lut_nodes_with_gia( lut_graph_t& lut, abc::Gia_Man_t* abc_lut, int k, std::unordered_map<int, lut_vertex_t>& lutid_to_node )
{
  int i;

  //auto tts = boost::get( boost::vertex_lut, lut );
  auto types = boost::get( boost::vertex_lut_type, lut );

  abc::Gia_ObjComputeTruthTableStart( abc_lut, k );
  Gia_ManForEachLut( abc_lut, i )
  {
    const auto size = abc::Gia_ObjLutSize( abc_lut, i );

    abc::Vec_Int_t leaves;
    leaves.nCap = leaves.nSize = size;
    leaves.pArray = abc::Gia_ObjLutFanins( abc_lut, i );

    //abc::Vec_IntSelectSort( abc::Vec_IntArray( &leaves ), abc::Vec_IntSize( &leaves ) );
    //const auto obj = abc::Gia_ManObj( abc_lut, i );
    //const auto* pt = abc::Gia_ObjComputeTruthTableCut( abc_lut, obj, &leaves );
    //abc::Vec_IntReverseOrder( &leaves );

    assert( false && "assign tts[v] with hex string of pt[0]" );

    auto v = add_vertex( lut );
    //tts[v] = std::make_pair( size, pt[0] );
    types[v] = lut_type_t::internal;

    for ( auto j = 0; j < size; ++j )
    {
      const auto it = lutid_to_node.find( leaves.pArray[j] );
      assert( it != lutid_to_node.end() );

      add_edge( v, it->second, lut );
    }

    lutid_to_node.insert( std::make_pair( i, v ) );
  }
  abc::Gia_ObjComputeTruthTableStop( abc_lut );
}

void map_lut_nodes_with_hop( lut_graph_t& lut, abc::Gia_Man_t* abc_lut, int k, std::unordered_map<int, lut_vertex_t>& lutid_to_node )
{
  int i;

  //auto tts = boost::get( boost::vertex_lut, lut );
  auto types = boost::get( boost::vertex_lut_type, lut );

  auto* hop = abc::Hop_ManStart();

  abc::Vec_Ptr_t* reflect = abc::Vec_PtrStart( abc::Gia_ManObjNum( abc_lut ) );
  abc::Vec_Int_t* memory  = abc::Vec_IntAlloc( 10000 );

  Gia_ManForEachLut( abc_lut, i )
  {
    const auto size = abc::Gia_ObjLutSize( abc_lut, i );

    auto* hop_obj = abc::Abc_ObjHopFromGia( hop, abc_lut, i, reflect );
    auto* truth   = abc::Hop_ManConvertAigToTruth( hop, abc::Hop_Regular( hop_obj ), size, memory, 0 );
    if ( abc::Hop_IsComplement( hop_obj ) )
    {
      abc::Extra_TruthNot( truth, truth, size );
    }

    assert( false && "assign tts[v] with hex value of truth[0]" );
    auto v = add_vertex( lut );
    //tts[v] = std::make_pair( size, truth[0] );
    types[v] = lut_type_t::internal;

    auto* fanin = abc::Gia_ObjLutFanins( abc_lut, i );
    for ( auto j = 0; j < size; ++j )
    {
      const auto it = lutid_to_node.find( fanin[j] );
      assert( it != lutid_to_node.end() );

      add_edge( v, it->second, lut );
    }

    lutid_to_node.insert( std::make_pair( i, v ) );
  }

  abc::Vec_IntFree( memory );
  abc::Vec_PtrFree( reflect );

  abc::Hop_ManStop( hop );
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

lut_graph_t abc_lut_mapping( const aig_graph& aig, unsigned k,
                             const properties::ptr& settings,
                             const properties::ptr& statistics )
{
  assert( k <= 6u ); /* so far */

  /* settings */
  const auto verbose = get( settings, "verbose", false );

  /* timing */
  properties_timer timer( statistics );

  /* compute ABC LUT mapping */
  auto* abc_lut = aig_mapping( aig, k );
  if ( verbose )
  {
    abc::Gia_ManPrintLutStats( abc_lut );
  }

  /* Setup LUT graph with primary inputs */
  lut_graph_t lut;
  std::unordered_map<int, lut_vertex_t> lutid_to_node;
  auto names = boost::get( boost::vertex_name, lut );
  auto types = boost::get( boost::vertex_lut_type, lut );
  //auto tts = boost::get( boost::vertex_lut, lut );

  int i, id;
  Gia_ManForEachCiId( abc_lut, id, i )
  {
    auto v = add_vertex( lut );
    lutid_to_node.insert( {id, v} );
    types[v] = lut_type_t::pi;
    names[v] = std::string( (char*)abc::Vec_PtrGetEntry( abc_lut->vNamesIn, i ) );
  }

  map_lut_nodes_with_gia( lut, abc_lut, k, lutid_to_node );

  Gia_ManForEachCoDriverId( abc_lut, id, i )
  {
    auto v = add_vertex( lut );
    types[v] = lut_type_t::po;
    names[v] = std::string( (char*)abc::Vec_PtrGetEntry( abc_lut->vNamesOut, i ) );

    const auto it = lutid_to_node.find( id );
    assert( it != lutid_to_node.end() );

    add_edge( v, it->second, lut );

    /* output inverted */
    if ( abc::Gia_ObjFaninC0( abc::Gia_ManCo( abc_lut, i ) ) == 1 )
    {
      assert( false && "fix inversion of truth table in hex format" );
      //tts[it->second].second = ~tts[it->second].second;
    }
  }

  abc::Gia_ManStop( abc_lut );

  return lut;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
