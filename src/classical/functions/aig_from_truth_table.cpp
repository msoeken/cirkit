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

#include "aig_from_truth_table.hpp"

#include <vector>

#include <boost/format.hpp>

#include <core/utils/bitset_utils.hpp>
#include <classical/abc/abc_api.hpp>
#include <classical/abc/abc_manager.hpp>
#include <classical/abc/functions/gia_to_cirkit.hpp>
#include <classical/utils/aig_utils.hpp>

#include <aig/gia/gia.h>

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

aig_graph aig_from_truth_table_naive( const tt& t )
{
  aig_graph aig;
  aig_initialize( aig );

  const auto n = tt_num_vars( t );

  for ( auto i = 1u; i <= n; ++i )
  {
    aig_create_pi( aig, boost::str( boost::format( "x%d" ) % i ) );
  }

  const auto& info = aig_info( aig );
  std::vector<aig_function> minterms;

  foreach_minterm( t, [&]( const boost::dynamic_bitset<>& minterm ) {
      std::vector<aig_function> cube;
      for ( auto i = 0u; i < n; ++i )
      {
        cube.push_back( {info.inputs[i], !minterm[i]} );
      }

      minterms.push_back( aig_create_nary_and( aig, cube ) );
    } );

  aig_create_po( aig, aig_create_nary_or( aig, minterms ), "f" );

  return aig;
}

aig_graph aig_from_truth_table( const tt& t )
{
  abc_manager::get();
  abc::Abc_FrameGetGlobalFrame();

  auto* sop_cover = abc::Abc_SopFromTruthBin( const_cast<char*>( to_string( t ).c_str() ) );
  auto* ntk = abc::Abc_NtkCreateWithNode( sop_cover );
  ABC_FREE( sop_cover );

  auto* ntk2 = abc::Abc_NtkStrash( ntk, 0, 1, 0 );
  abc::Abc_NtkDelete( ntk );

  auto* aig = abc::Abc_NtkToDar( ntk2, 0, 0 );
  abc::Abc_NtkDelete( ntk2 );

  auto* gia = abc::Gia_ManFromAig( aig );
  abc::Aig_ManStop( aig );

  /* simple */
  //abc::Abc_NtkToAig( ntk );
  //auto * gia = abc::Abc_NtkAigToGia( ntk, 1 );

  return gia_to_cirkit( gia );
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
