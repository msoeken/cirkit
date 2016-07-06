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
#include <classical/abc/utils/abc_run_command.hpp>
#include <classical/abc/functions/gia_to_cirkit.hpp>
#include <classical/utils/aig_utils.hpp>

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
  return abc_run_command( boost::str( boost::format( "read_truth -x %s; strash; dc2; &get" ) % to_string( t )  ) );
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
