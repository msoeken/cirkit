/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
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

#include "truth_table_from_bitset.hpp"

#include <classical/utils/truth_table_utils.hpp>

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

binary_truth_table truth_table_from_bitset( const boost::dynamic_bitset<>& bs )
{
  const auto num_vars = tt_num_vars( bs );
  const auto bw = ( ( bs.count() << 1u ) == bs.size() ) ? num_vars : num_vars + 1u;

  binary_truth_table spec;

  for ( auto i = 0u; i < ( 1u << bw ); ++i )
  {
    auto in = number_to_truth_table_cube( i, bw );
    binary_truth_table::cube_type out( bw, boost::optional<bool>() );
    if ( i < bs.size() )
    {
      out[0u] = bs[i];
    }
    spec.add_entry( in, out );
  }

  return spec;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
