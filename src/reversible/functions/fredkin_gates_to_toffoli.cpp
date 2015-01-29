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

#include "fredkin_gates_to_toffoli.hpp"

#include <boost/assign/std/vector.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>

#include <reversible/target_tags.hpp>
#include <reversible/functions/add_gates.hpp>
#include <reversible/functions/copy_metadata.hpp>

using namespace boost::assign;

namespace cirkit
{

void fredkin_gates_to_toffoli( const circuit& src, circuit& dest )
{
  copy_metadata( src, dest );

  for ( const auto& g : src )
  {
    if ( is_fredkin( g ) )
    {
      gate::control_container controls;
      boost::push_back( controls, g.controls() );
      controls += make_var( g.targets()[0u], true );

      append_cnot( dest, g.targets()[1u], g.targets()[0u] );
      append_toffoli( dest, controls, g.targets()[1u] );
      append_cnot( dest, g.targets()[1u], g.targets()[0u] );
    }
    else if ( is_toffoli( g ) )
    {
      dest.append_gate() = g;
    }
    else
    {
      assert( false );
    }
  }
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
