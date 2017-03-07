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

#include "expand_circuit.hpp"

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
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
