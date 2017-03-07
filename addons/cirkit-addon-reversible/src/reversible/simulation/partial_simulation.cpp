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

#include "partial_simulation.hpp"

#include <core/utils/timer.hpp>

#include <reversible/circuit.hpp>
#include <reversible/gate.hpp>

#include <reversible/simulation/simple_simulation.hpp>

namespace cirkit
{

  bool partial_simulation( boost::dynamic_bitset<>& output, const circuit& circ, const boost::dynamic_bitset<>& input, properties::ptr settings, properties::ptr statistics )
  {
    simulation_func simulation = get<simulation_func>( settings, "simulation", simple_simulation_func() );
    bool keep_full_output = get<bool>( settings, "keep_full_output", false );

    properties_timer t( statistics );

    boost::dynamic_bitset<> full_input( circ.lines(), 0ul );
    
    unsigned input_pos = 0u;

    for ( unsigned i = 0u; i < circ.lines(); ++i )
    {
      constant con = circ.constants().at( i );

      full_input.set( i, con ? *con : input.test( input_pos ) );
      if ( !con )
      {
        ++input_pos;
      }
    }

    boost::dynamic_bitset<> full_output;

    simulation( full_output, circ, full_input );

    if ( keep_full_output )
    {
      output = full_output;
    }
    else
    {
      output.resize( std::count( circ.garbage().begin(), circ.garbage().end(), false ) );

      unsigned output_pos = 0u;
      for ( unsigned i = 0; i < full_output.size(); ++i )
      {
        if ( !circ.garbage().at( i ) )
        {
          output.set( output_pos++, full_output.test( i ) );
        }
      }
    }

    return true;
  }

  simulation_func partial_simulation_func( properties::ptr settings, properties::ptr statistics )
  {
    simulation_func f = [&settings, &statistics]( boost::dynamic_bitset<>& output, const circuit& circ, const boost::dynamic_bitset<>& input ) {
      return partial_simulation( output, circ, input, settings, statistics );
    };
    f.init( settings, statistics );
    return f;
  }

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
