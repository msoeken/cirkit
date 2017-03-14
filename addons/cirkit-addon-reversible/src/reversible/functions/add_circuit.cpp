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

#include "add_circuit.hpp"

#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext/insert.hpp>

namespace cirkit
{

  void append_circuit( circuit& circ, const circuit& src, const gate::control_container& controls, const std::vector<unsigned>& line_map )
  {
    insert_circuit( circ, circ.num_gates(), src, controls, line_map );
  }

  void prepend_circuit( circuit& circ, const circuit& src, const gate::control_container& controls, const std::vector<unsigned>& line_map )
  {
    insert_circuit( circ, 0, src, controls, line_map );
  }

  void insert_circuit( circuit& circ, unsigned pos, const circuit& src, const gate::control_container& controls, const std::vector<unsigned>& line_map )
  {
    if ( controls.empty() && line_map.empty() )
    {
      for ( const auto& g : src )
      {
        circ.insert_gate( pos++ ) = g;
      }
    }
    else
    {
      for ( const auto& g : src )
      {
        gate& new_gate = circ.insert_gate( pos++ );
        boost::for_each( controls, [&new_gate](variable c) { new_gate.add_control( c ); } );
        if ( line_map.empty() )
        {
          boost::for_each( g.controls(), [&new_gate](variable c) { new_gate.add_control( c ); } );
          boost::for_each( g.targets(), [&new_gate](unsigned t) { new_gate.add_target( t ); } );
        }
        else
        {
          boost::for_each( g.controls(), [&new_gate, &line_map](variable c) { new_gate.add_control( make_var( line_map[c.line()], c.polarity() ) ); } );
          boost::for_each( g.targets(), [&new_gate, &line_map](unsigned t) { new_gate.add_target( line_map[t] ); } );
        }
        new_gate.set_type( g.type() );
      }
    }
  }

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
