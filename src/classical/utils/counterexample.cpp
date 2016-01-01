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

#include "counterexample.hpp"

namespace cirkit
{

std::ostream& operator<<( std::ostream& os, const assignment_t& a )
{
  assert( a.bits.size() == a.mask.size() );
  const unsigned size = a.bits.size();
  for ( unsigned i = 0u; i < size; ++i )
  {
    os << ( a.mask[i] ? ( a.bits[i] ? '1' : '0' ) : 'X' );
  }
  return os;
}

std::ostream& operator<<( std::ostream& os, const counterexample_t& cex )
{
  os << cex.in << '|' << cex.out << '-' << cex.expected_out;
  return os;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
