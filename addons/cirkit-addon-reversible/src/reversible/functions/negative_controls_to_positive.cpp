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

#include "negative_controls_to_positive.hpp"

#include <reversible/functions/add_gates.hpp>
#include <reversible/functions/copy_metadata.hpp>


namespace cirkit
{

void negative_controls_to_positive( const circuit& src, circuit& dest )
{
  copy_metadata( src, dest );

  for ( const auto& g : src )
  {
    for ( const auto& n : g.controls() )
    {
      if ( !n.polarity() )
      {
        append_not( dest, n.line() );
      }
    }

    gate& ng = dest.append_gate();
    for ( const auto& c : g.controls() ) { ng.add_control( make_var( c.line() ) ); }
    for ( const auto& t : g.targets() )  { ng.add_target( t );                     }
    ng.set_type( g.type() );

    for ( const auto& n : g.controls() )
    {
      if ( !n.polarity() )
      {
        append_not( dest, n.line() );
      }
    }
  }
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
