/* CirKit: A circuit toolkit
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

#include "print_circuit.hpp"

#include <boost/algorithm/string/trim.hpp>
#include <boost/format.hpp>
#include <boost/iterator/counting_iterator.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/irange.hpp>

#include <reversible/circuit.hpp>
#include <reversible/target_tags.hpp>

namespace cirkit
{

  print_circuit_settings::print_circuit_settings( std::ostream& os )
    : os( os ),
      print_inputs_and_outputs( false ),
      print_gate_index( false ),
      control_char( "●" ),
      negative_control_char( "○" ),
      line_char( "―" ),
      gate_spacing( 1u ),
      line_spacing( 0u )
  {
  }

  std::string print_circuit_settings::target_type_char( const gate& g ) const
  {
    if ( is_toffoli( g ) || is_peres( g ) )
    {
      return "⊕";
    }
    else if ( is_fredkin( g ) )
    {
      return  "⨯";
    }
    else if ( is_module( g ) )
    {
      return "□";
    }
    else
    {
      return "⁈";
    }
  }

  struct string_size_compare
  {
    bool operator()( const std::string& a, const std::string& b ) const
    {
      return a.size() < b.size();
    }
  };

  void print_circuit( const circuit& circ, const print_circuit_settings& settings )
  {
    if ( circ.num_gates() == 0 || circ.lines() == 0 )
    {
      return;
    }

    // when printing inputs and outputs we need to find the maximum size
    unsigned longest_input_length = 0;

    if ( settings.print_inputs_and_outputs )
    {
      longest_input_length = std::max_element( circ.inputs().begin(), circ.inputs().end(), string_size_compare() )->size();
    }

    // each element is a line with elements of (gate_index,character)
    std::vector<std::vector<std::tuple<unsigned, std::string> > > line_chars( circ.lines() );

    // iterate over all gates
    for ( circuit::const_iterator itGate = circ.begin(); itGate != circ.end(); ++itGate )
    {
      unsigned gate_index = itGate - circ.begin();

      // iterate over all controls in the gate
      for ( const auto& v : itGate->controls() )
      {
        if ( v.line() < line_chars.size() )
        {
          line_chars[v.line()].push_back( std::make_tuple( gate_index, v.polarity() ? settings.control_char : settings.negative_control_char ) );
        }
      }

      // iterate over all targets in the gate
      for ( const auto& l : itGate->targets() )
      {
        if ( l < line_chars.size() )
        {
          line_chars[l].push_back( std::make_tuple( gate_index, settings.target_type_char( *itGate ) ) );
        }
      }
    }

    if ( settings.print_gate_index )
    {
      if ( settings.print_inputs_and_outputs )
      {
        settings.os << std::string( longest_input_length + 1, ' ' );
      }

      boost::for_each( boost::irange( 0u, circ.num_gates() ), [&settings]( unsigned n ) { settings.os << ( n % 10 ); } );
      settings.os << std::endl;
    }

    for ( unsigned i = 0; i < circ.lines(); ++i )
    {
      using boost::adaptors::indexed;
      std::string line_str;

      auto add_line_char = [&line_str, &settings]( unsigned from, unsigned to ) {
        for ( unsigned j = from; j < to; ++j )
          for ( unsigned k = 0u; k <= settings.gate_spacing; ++k )
            line_str += settings.line_char;
      };

      unsigned pos = 0u;
      for ( const auto& lc : line_chars.at( i ) )
      {
        add_line_char( pos, std::get<0>( lc ) );
        for ( unsigned k = 0u; k < settings.gate_spacing; ++k ) line_str += settings.line_char;
        line_str += std::get<1>( lc );
        pos = std::get<0>( lc ) + 1u;
      }
      add_line_char( pos, circ.num_gates() );
      for ( unsigned k = 0u; k < settings.gate_spacing; ++k ) line_str += settings.line_char;

      if ( settings.print_inputs_and_outputs )
      {
        settings.os << std::string( longest_input_length - circ.inputs().at( i ).size(), ' ' ) << circ.inputs().at( i ) << " ";
      }

      settings.os << line_str;

      if ( settings.print_inputs_and_outputs )
      {
        settings.os << " " << circ.outputs().at( i );
      }

      settings.os << std::endl;

      for ( unsigned j = 0; j < settings.line_spacing; ++j )
      {
        settings.os << std::endl;
      }
    }
  }

  std::ostream& operator<<( std::ostream& os, const circuit& circ )
  {
    print_circuit_settings settings( os );
    print_circuit( circ, settings );
    return os;
  }

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
