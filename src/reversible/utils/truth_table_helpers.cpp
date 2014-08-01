/* RevKit (www.rekit.org)
 * Copyright (C) 2009-2014  University of Bremen
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

#include "truth_table_helpers.hpp"

#include <boost/range/iterator_range.hpp>

namespace cirkit
{

void truth_table_to_bitset_vector( const binary_truth_table& spec, bitset_vector_t& vec )
{
  /* Number of variables */
  unsigned n = spec.num_inputs();

  /* Resize vector */
  vec.resize( 1u << n );

  /* Fill vector */
  for ( const auto& row : spec )
  {
    boost::dynamic_bitset<> inv( n ), outv( n );

    unsigned pos = 0u;
    for ( const auto& in : boost::make_iterator_range( row.first ) )
    {
      inv[n - pos++ - 1u] = *in;
    }

    pos = 0u;
    for ( const auto& out : boost::make_iterator_range( row.second ) )
    {
      outv[n - pos++ - 1u] = *out;
    }

    vec[inv.to_ulong()] = outv;
  }
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
