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

#include "expand_circuit.hpp"

#include <boost/tuple/tuple.hpp>

#include <reversible/functions/copy_circuit.hpp>

namespace cirkit
{

  bool expand_circuit( const circuit& base, circuit& circ, unsigned num_lines, const std::vector<unsigned>& filter )
  {
    /* circ has to be empty */
    if ( circ.num_gates() != 0 )
    {
      assert( false );
      return false;
    }

    if ( num_lines == 0u || !filter.size() )
    {
      copy_circuit( base, circ );
      return true;
    }
    else
    {
      circ.set_lines( num_lines );

      for ( const auto& g : base )
      {
        gate& new_g = circ.append_gate();

        for ( const auto& v : g.controls() )
        {
          new_g.add_control( make_var( filter.at( v.line() ), v.polarity() ) );
        }

        for ( const auto& l : g.targets() )
        {
          new_g.add_target( filter.at( l ) );
        }

        new_g.set_type( g.type() );
      }
    }

    return true;
  }

}

// Local Variables:
// c-basic-offset: 2
// End:
