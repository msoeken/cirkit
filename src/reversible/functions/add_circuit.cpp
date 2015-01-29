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

#include "add_circuit.hpp"

#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext/insert.hpp>

namespace cirkit
{

  void append_circuit( circuit& circ, const circuit& src, const gate::control_container& controls )
  {
    insert_circuit( circ, circ.num_gates(), src, controls );
  }

  void prepend_circuit( circuit& circ, const circuit& src, const gate::control_container& controls )
  {
    insert_circuit( circ, 0, src, controls );
  }

  void insert_circuit( circuit& circ, unsigned pos, const circuit& src, const gate::control_container& controls )
  {
    if ( controls.empty() )
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
        boost::for_each( g.controls(), [&new_gate](variable c) { new_gate.add_control( c ); } );
        boost::for_each( g.targets(), [&new_gate](unsigned t) { new_gate.add_target( t ); } );
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
