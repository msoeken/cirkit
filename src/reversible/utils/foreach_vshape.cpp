/* RevKit (www.revkit.org)
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

#include "foreach_vshape.hpp"

#include <boost/assign/std/vector.hpp>

#include <core/utils/bitset_utils.hpp>
#include <reversible/circuit.hpp>
#include <reversible/functions/add_gates.hpp>

namespace cirkit
{

void foreach_vshape( unsigned n, const std::function<void(const circuit&)>& f )
{
  using namespace boost::assign;

  boost::dynamic_bitset<> range( (2u * n - 1u) * (n - 1u) );
  do
  {
    circuit circ( n );

    for ( unsigned i = 0u; i < 2u * n - 1u; ++i )
    {
      unsigned t = i < n ? i : (2u * n - 2u - i);
      unsigned offset = i * ( n - 1u );

      gate::control_container controls;
      for ( unsigned j = 0u; j < n - 1u; ++j )
      {
        unsigned c = j < t ? j : j + 1u;
        if ( range[offset + j] )
        {
          controls += make_var( c );
        }
      }
      append_toffoli( circ, controls, t );
    }

    f( circ );

    inc( range );
  } while ( !range.none() );
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
