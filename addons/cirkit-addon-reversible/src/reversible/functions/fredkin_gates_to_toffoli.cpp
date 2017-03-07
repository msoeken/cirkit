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
