/* RevKit: A Toolkit for Reversible Circuit Design (www.revkit.org)
 * Copyright (C) 2009-2013  The RevKit Developers <revkit@informatik.uni-bremen.de>
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

#include "copy_circuit.hpp"

#include <algorithm>

#include <boost/range/algorithm.hpp>
#include <boost/tuple/tuple.hpp>

#include "../circuit.hpp"

#include "add_circuit.hpp"
#include "copy_metadata.hpp"

namespace cirkit
{

  void copy_circuit( const circuit& src, circuit& dest )
  {
    assert( !dest.num_gates() );
    assert( !dest.lines() );

    append_circuit( dest, src );
    copy_metadata( src, dest );
  }

  void copy_circuit( const circuit& src, circuit& dest, const std::vector<unsigned>& filter )
  {
    // Some pre-conditions
    assert( !dest.num_gates() );
    assert( !dest.lines() );

    boost::for_each( filter, [&src]( unsigned line ) { assert( line < src.lines() ); } );

    dest.set_lines( filter.size() );

    // Copy gate by gate
    for ( const auto& g : src )
    {
      // Add empty gate
      gate& ng = dest.append_gate();

      // Copy control and target lines and set same type
      for ( variable c : g.controls() )
      {
        auto match = boost::find( filter, c.line() );
        if ( match != filter.end() )
        {
          ng.add_control( make_var( std::distance( filter.begin(), match ), c.polarity() ) );
        }
      }
      for ( unsigned t : g.targets() )
      {
        auto match = boost::find( filter, t );
        if ( match != filter.end() )
        {
          ng.add_target( std::distance( filter.begin(), match ) );
        }
      }
      ng.set_type( g.type() );
    }
  }

}

// Local Variables:
// c-basic-offset: 2
// End:
