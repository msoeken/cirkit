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

#include "simple_simulation.hpp"

#include <core/utils/timer.hpp>

#include <reversible/gate.hpp>
#include <reversible/target_tags.hpp>

namespace cirkit
{

  boost::dynamic_bitset<>& core_gate_simulation::operator()( const gate& g, boost::dynamic_bitset<>& input ) const
  {
    if ( is_toffoli( g ) )
    {
      boost::dynamic_bitset<> c_mask( input.size() );
      boost::dynamic_bitset<> input_copy = input;
      for ( const auto& v : g.controls() )
      {
        if ( !v.polarity() )
        {
          input_copy.flip( v.line() );
        }
        c_mask.set( v.line() );
      }

      if ( c_mask.none() || ( ( input_copy & c_mask ) == c_mask ) )
      {
        input.flip( g.targets().front() );
      }

      return input;
    }
    // TODO negative controls
    else if ( is_fredkin( g ) )
    {
      boost::dynamic_bitset<> c_mask( input.size() );
      for ( const auto& v : g.controls() )
      {
        assert( v.polarity() );
        c_mask.set( v.line() );
      }

      if ( c_mask.none() || ( ( input & c_mask ) == c_mask ) )
      {
        // get both positions and values
        unsigned t1 = g.targets().at( 0u );
        unsigned t2 = g.targets().at( 1u );

        bool t1v = input.test( t1 );
        bool t2v = input.test( t2 );

        // only swap when different
        if ( t1v != t2v )
        {
          input.set( t1, t2v );
          input.set( t2, t1v );
        }
      }

      return input;
    }
    else if ( is_peres( g ) )
    {
      if ( input.test( g.controls().front().line() ) ) // is single control set
      {
        // get both positions and value of t1
        unsigned t1 = g.targets().at( 0u );
        unsigned t2 = g.targets().at( 1u );

        bool t1v = input.test( t1 );

        /* flip t1 */
        input.flip( t1 );

        /* flip t2 if t1 was true */
        if ( t1v )
        {
          input.flip( t2 );
        }
      }

      return input;
    }
    else if ( is_module( g ) )
    {
      boost::dynamic_bitset<> c_mask( input.size() );
      for ( const auto& v : g.controls() )
      {
        assert( v.polarity() );
        c_mask.set( v.line() );
      }

      // cancel if controls are not hit
      if ( !c_mask.is_subset_of( input ) )
      {
        return input;
      }

      const module_tag* tag = boost::any_cast<module_tag>( &g.type() );

      // TODO write test case
      // get the new input sub pattern
      boost::dynamic_bitset<> tpattern( g.targets().size() );
      for ( const auto& i : g.targets() )
      {
        tpattern.set( i, input.test( i ) );
      }
      boost::dynamic_bitset<> toutput;
      assert( simple_simulation( toutput, *tag->reference, tpattern ) );

      for ( const auto& i : g.targets() )
      {
        input.set( i, toutput.test( i ) );
      }

      return input;
    }
    else if ( is_stg( g ) )
    {
      const auto stg = boost::any_cast<stg_tag>( g.type() );

      boost::dynamic_bitset<> f_pattern( g.controls().size() );
      for ( auto i = 0u; i < g.controls().size(); ++i )
      {
        if ( input.test( g.controls()[i].line() ) )
        {
          f_pattern.set( i );
        }
      }
      if ( stg.function.test( f_pattern.to_ulong() ) )
      {
        input.flip( g.targets().front() );
      }
      return input;
    }
    else
    {
      assert( false );
      return input;
    }
  }

  bool simple_simulation( boost::dynamic_bitset<>& output, const gate& g, const boost::dynamic_bitset<>& input,
                          properties::ptr settings,
                          properties::ptr statistics )
  {
    gate_simulation_func gate_simulation = get<gate_simulation_func>( settings, "gate_simulation", core_gate_simulation() );
    step_result_func     step_result     = get<step_result_func>( settings, "step_result", step_result_func() );

    properties_timer t( statistics );

    output = input;
    output = gate_simulation( g, output );
    if ( step_result )
    {
      step_result( g, output );
    }
    return true;
  }

  bool simple_simulation( boost::dynamic_bitset<>& output, circuit::const_iterator first, circuit::const_iterator last, const boost::dynamic_bitset<>& input,
                          properties::ptr settings,
                          properties::ptr statistics )
  {
    gate_simulation_func gate_simulation = get<gate_simulation_func>( settings, "gate_simulation", core_gate_simulation() );
    step_result_func     step_result     = get<step_result_func>( settings, "step_result", step_result_func() );

    properties_timer t( statistics );

    output = input;
    while ( first != last )
    {
      output = gate_simulation( *first, output );
      if ( step_result )
      {
        step_result( *first, output );
      }
      ++first;
    }
    return true;
  }

  bool simple_simulation( boost::dynamic_bitset<>& output, const circuit& circ, const boost::dynamic_bitset<>& input,
                          properties::ptr settings,
                          properties::ptr statistics )
  {
    return simple_simulation( output, circ.begin(), circ.end(), input, settings, statistics );
  }

  simulation_func simple_simulation_func( properties::ptr settings, properties::ptr statistics )
  {
    simulation_func f = [&settings, &statistics]( boost::dynamic_bitset<>& output, const circuit& circ, const boost::dynamic_bitset<>& input ) {
      return simple_simulation( output, circ, input, settings, statistics );
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
