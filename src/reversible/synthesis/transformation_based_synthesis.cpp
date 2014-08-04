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

#include "transformation_based_synthesis.hpp"

#include <core/utils/timer.hpp>
#include <reversible/circuit.hpp>
#include <reversible/functions/add_gates.hpp>
#include <reversible/functions/clear_circuit.hpp>
#include <reversible/functions/copy_metadata.hpp>
#include <reversible/functions/fully_specified.hpp>
#include <reversible/io/print_circuit.hpp>

#include "synthesis_utils_p.hpp"

namespace cirkit
{

  void read_output_values( const binary_truth_table& spec, std::vector<unsigned>& output_values )
  {
    unsigned bw = spec.num_outputs();

    std::map<unsigned, unsigned> value_map;

    for ( binary_truth_table::const_iterator it = spec.begin(); it != spec.end(); ++it )
    {
      binary_truth_table::in_const_iterator ci = it->first.first;
      binary_truth_table::out_const_iterator c = it->second.first;

      unsigned output = 0;
      for ( unsigned i = 0; i < bw; ++i )
      {
        output |= ( **( c + i ) << ( bw - 1 - i ) ); // iterator deref -> boost::optional deref
      }

      unsigned input = 0;
      for ( unsigned i = 0; i < bw; ++i )
      {
        input |= ( **( ci + i ) << ( bw - 1 - i ) ); // iterator deref -> boost::optional deref
      }

      value_map[input] = output;
    }

    for ( std::map<unsigned, unsigned>::const_iterator it = value_map.begin(); it != value_map.end(); ++it )
    {
      output_values.push_back( it->second );
    }
  }

  void get_control_lines_from_mask( unsigned mask, unsigned bw, gate::control_container& controls )
  {
    for ( unsigned j = 0; j < bw; ++j )
    {
      if ( mask & ( 1u << ( bw - 1 - j ) ) )
      {
        controls.push_back( make_var( j ) );
      }
    }
  }

  void insert_gate( circuit& circ, unsigned& pos, unsigned mask, unsigned target_line, unsigned current_line, std::vector<unsigned>& output_values )
  {
    gate::control_container controls;
    get_control_lines_from_mask( mask, circ.lines(), controls );

    insert_toffoli( circ, pos, controls, target_line );

    for ( unsigned i = current_line; i < output_values.size(); ++i )
    {
      if ( ( output_values.at( i ) & mask ) == mask )
      {
        output_values[i] ^= ( 1u << ( circ.lines() - 1 - target_line ) );
      }
    }
  }

  void insert_gate_front( circuit& circ, unsigned& pos, unsigned mask, unsigned target_line, unsigned current_line, std::vector<unsigned>& output_values )
  {
    gate::control_container controls;
    get_control_lines_from_mask( mask, circ.lines(), controls );

    insert_toffoli( circ, pos, controls, target_line );

    for ( unsigned i = current_line; i < output_values.size(); ++i )
    {
      if ( ( i & mask ) == mask )
      {
        unsigned otherpos = i ^ ( 1u << ( circ.lines() - 1 - target_line ) );
        if ( otherpos > i )
        {
          unsigned tmp = output_values.at( i );
          output_values[i] = output_values.at( otherpos );
          output_values[otherpos] = tmp;
        }
      }
    }
  }

  void basic_first_step( circuit& circ, std::vector<unsigned>& output_values )
  {
    unsigned bw = circ.lines();

    for ( unsigned i = 0; i < bw; ++i )
    {
      unsigned num_pos = bw - 1 - i;
      bool bit = output_values.at( 0 ) & ( 1u << num_pos );

      if ( bit )
      {
        prepend_not( circ, i );
        for ( unsigned& ov : output_values )
        {
          ov ^= ( 1u << num_pos );
        }
      }
    }
  }

  void insert_from_back( circuit& circ, unsigned& pos, unsigned line, std::vector<unsigned>& output_values )
  {
    unsigned bw = circ.lines();

    // change 0 -> 1
    if ( unsigned p = ( line ^ output_values.at( line ) ) & line )
    {
      for ( unsigned j = 0; j < bw; ++j )
      {
        // if p_j is set
        if ( p & ( 1u << ( bw - 1 - j ) ) )
        {
          unsigned mask = output_values.at( line ) & ~( 1u << ( bw - 1 - j ) );
          insert_gate( circ, pos, mask, j, line, output_values );
        }
      }
    }

    // change 1 -> 0
    if ( unsigned q = ( line ^ output_values.at( line ) ) & output_values.at( line ) )
    {
      for ( unsigned j = 0; j < bw; ++j )
      {
        // if q_j is set
        if ( q & ( 1u << ( bw - 1 - j ) ) )
        {
          //          unsigned mask = line & ~( 1u << ( bw - 1 - j ) );
          unsigned mask = output_values.at( line ) & ~( 1u << ( bw - 1 - j ) );
          insert_gate( circ, pos, mask, j, line, output_values );
        }
      }
    }
  }

  void insert_from_front( circuit& circ, unsigned& pos, unsigned line, std::vector<unsigned>& output_values )
  {
    unsigned bw = circ.lines();

    while ( line != output_values.at( line ) )
    {
      unsigned p = ( line ^ output_values.at( line ) ) & output_values.at( line );
      unsigned q = ( line ^ output_values.at( line ) ) & line;

      unsigned value = output_values.at( line );

      // change 0 -> 1
      if ( p )
      {
        for ( unsigned j = 0; j < bw; ++j )
        {
          // if p_j is set
          if ( p & ( 1u << ( bw - 1 - j ) ) )
          {
            //            unsigned mask = output_values.at( line ) & ~( 1u << ( bw - 1 - j ) );
            unsigned mask = line & ~( 1u << ( bw - 1 - j ) );
            insert_gate_front( circ, pos, mask, j, 0, output_values );
            ++pos;
            break;
          }
        }
      }

      // change 1 -> 0
      if ( !p && q )
      {
        for ( unsigned j = 0; j < bw; ++j )
        {
          // if q_j is set
          if ( q & ( 1u << ( bw - 1 - j ) ) )
          {
            unsigned mask = line & ~( 1u << ( bw - 1 - j ) );
            insert_gate_front( circ, pos, mask, j, 0, output_values );
            ++pos;
            break;
          }
        }
      }

      line = std::find( output_values.begin(), output_values.end(), value ) - output_values.begin();
    }
  }

  bool transformation_based_synthesis( circuit& circ, const binary_truth_table& spec,
                                       properties::ptr settings,
                                       properties::ptr statistics )
  {
    /* Settings */
    bool bidirectional = get( settings, "bidirectional", true );

    timer<properties_timer> t;

    if ( statistics )
    {
      properties_timer rt( statistics ); 
      t.start( rt );
    }

    // circuit has to be empty
    clear_circuit( circ );

    // truth table has to be fully specified
    if ( !fully_specified( spec ) )
    {
      set_error_message( statistics, "truth table `spec` is not fully specified." );
      return false;
    }

    std::vector<unsigned> output_values;
    read_output_values( spec, output_values );

    unsigned bw = spec.num_outputs();
    circ.set_lines( bw );

    // copy metadata
    copy_metadata( spec, circ );

    // Step 1
    if ( !bidirectional )
    {
      basic_first_step( circ, output_values );
    }

    // Step 2
    unsigned start_index = bidirectional ? 0 : 1;
    unsigned pos = 0;

    bool from_back = true;
    unsigned index = 0;

    for ( unsigned i = start_index; i < output_values.size(); ++i )
    {
      if ( i == output_values.at( i ) )
      {
        continue;
      }

      // NOTE maybe have two for loops so that the check has no to be done every time
      if ( bidirectional )
      {
        index = std::find( output_values.begin(), output_values.end(), i ) - output_values.begin();
        from_back = ( hamming_distance( index, output_values.at( index ) ) >= hamming_distance( i, output_values.at( i ) ) );
      }

      if ( from_back )
      {
        //        std::cout << "insert from back (line: " << i << ")" << std::endl;
        insert_from_back( circ, pos, i, output_values );
      }
      else
      {
        //        std::cout << "insert from font (line: " << i << ", index: " << index << ")" << std::endl;
        insert_from_front( circ, pos, index, output_values );
      }
    }

    return true;
  }

  truth_table_synthesis_func transformation_based_synthesis_func( properties::ptr settings,
                                                                  properties::ptr statistics )
  {
    truth_table_synthesis_func f = [&settings, &statistics]( circuit& circ, const binary_truth_table& spec ) {
      return transformation_based_synthesis( circ, spec, settings, statistics );
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
