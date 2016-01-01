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

#include "range_utils.hpp"

namespace cirkit
{

void mixed_radix( std::vector<unsigned>& a, const std::vector<unsigned>& m, const std::function<bool(const std::vector<unsigned>&)>&& func )
{
  while ( true )
  {
    /* step M2. [Visit.] */
    if ( func( a ) ) { return; }

    /* next iteration */
    auto j = a.size() - 1u;
    while ( a[j] == m[j] - 1u ) { a[j--] = 0u; }

    if ( !j ) { break; }

    a[j]++;
  }
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
