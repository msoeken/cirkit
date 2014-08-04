/* RevKit (www.revkit.org)
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

#include "pattern_to_circuit.hpp"
#include <iostream>
#include <boost/assign/std/vector.hpp>

#include "../gate.hpp"

#include "add_circuit.hpp"
#include "add_gates.hpp"
#include "reverse_circuit.hpp"

using namespace boost::assign;

namespace cirkit
{
  bool pattern_to_circuit( circuit& circ, const boost::dynamic_bitset<>& pattern1, const boost::dynamic_bitset<>& pattern2 )
  {
    assert( pattern1.size() == pattern2.size() );
    assert( circ.lines() == pattern1.size() );

    boost::dynamic_bitset<> pattern_and; 
    pattern_and = pattern1 ^ pattern2;   // bitwise xor
    unsigned last_position = 0u;                    // last position where the bits are different
    bool last_position_polarity = false; 
    for(unsigned i = 1; i <= pattern_and.size(); i++)
    {
       if(pattern_and[pattern_and.size() - i] == 1u)
       {
         last_position = pattern_and.size() - i;
	 last_position_polarity = (pattern1[pattern_and.size() - i] == 1u);
         // last_position_polarity = (pattern1[pattern1.size() - 1u] == 1u);
         std::cout << "polarity" << last_position_polarity << std::endl; 
	 break;
       }
    }

     for(unsigned i = 0; i < pattern_and.size(); ++i)
     {
       std::cout << "pattern_and[" << i << "] = " << pattern_and[i] << std::endl;    
     }

     std::cout << "last position = " << last_position << std::endl;



    for(unsigned i = 0; i < last_position; ++i)
    {
      gate::control_container controls;
      if(pattern_and[i] == 1u)
      {
        controls += make_var(last_position, last_position_polarity);
        // controls += make_var(pattern1.size() - 1u, last_position_polarity);
	append_toffoli(circ, controls, i);
      }
    }

    gate::control_container controls;
    //for(unsigned i = 0; i < last_position; i++)
    for(unsigned i = 0; i < pattern2.size(); i++)
    {
      if (i != last_position)
       controls += make_var(i, pattern2[i] == 1u);     
    }
    append_toffoli(circ, controls, last_position);


    for(unsigned i=1; i <= last_position; i++)
    {
      gate::control_container controls;
      if(pattern_and[last_position - i] == 1u)
      {
        controls += make_var(last_position, last_position_polarity);
        // controls += make_var(pattern1.size() -1u, last_position_polarity);
	append_toffoli(circ, controls, last_position - i);
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
