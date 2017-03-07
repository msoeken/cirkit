/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2017  EPFL
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "copy_circuit.hpp"

#include <algorithm>

#include <boost/range/algorithm.hpp>

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
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
