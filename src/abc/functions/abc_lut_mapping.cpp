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

#include "abc_lut_mapping.hpp"

#include <unordered_map>

#include <core/utils/timer.hpp>
#include <classical/utils/truth_table_utils.hpp>
#include <abc/abc_api.hpp>
#include <abc/functions/cirkit_to_gia.hpp>

#include <map/if/if.h>

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
  auto tts = boost::get( boost::vertex_lut, lut );
  auto types = boost::get( boost::vertex_gate_type, lut );

  int i, id;
  Gia_ManForEachCiId( abc_lut, id, i )
  {
    auto v = add_vertex( lut );
    lutid_to_node.insert( {id, v} );
    types[v] = gate_type_t::pi;
  }

  abc::Gia_ObjComputeTruthTableStart( abc_lut, k );
  Gia_ManForEachLut( abc_lut, i )
  {
    const auto size = abc::Gia_ObjLutSize( abc_lut, i );

    abc::Vec_Int_t leaves;
    leaves.nCap = leaves.nSize = size;
    leaves.pArray = abc::Gia_ObjLutFanins( abc_lut, i );

    const auto* pt = abc::Gia_ObjComputeTruthTableCut( abc_lut, abc::Gia_ManObj( abc_lut, i ), &leaves );

    auto v = add_vertex( lut );
    tts[v] = std::make_pair( size, pt[0] );
    types[v] = gate_type_t::internal;

    for ( auto j = 0; j < size; ++j )
    {
      const auto it = lutid_to_node.find( leaves.pArray[j] );
      assert( it != lutid_to_node.end() );

      add_edge( v, it->second, lut );
    }

    lutid_to_node.insert( std::make_pair( i, v ) );
  }

  Gia_ManForEachCoDriverId( abc_lut, id, i )
  {
    auto v = add_vertex( lut );
    types[v] = gate_type_t::po;

    const auto it = lutid_to_node.find( id );
    assert( it != lutid_to_node.end() );

    add_edge( v, it->second, lut );
  }

  abc::Gia_ObjComputeTruthTableStop( abc_lut );
  abc::Gia_ManStop( abc_lut );

  return lut;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
