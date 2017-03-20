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

#include "print_circuit.hpp"

#include <vector>

#include <boost/algorithm/string/trim.hpp>
#include <boost/format.hpp>
#include <boost/iterator/counting_iterator.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/counting_range.hpp>

#include <reversible/circuit.hpp>
#include <reversible/pauli_tags.hpp>
#include <reversible/target_tags.hpp>

namespace cirkit
{

print_circuit_settings::print_circuit_settings( std::ostream& os )
  : os( os )
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
    return "M";
  }
  else if ( is_stg( g ) )
  {
    return "⊕";
  }
  else if ( is_hadamard( g ) )
  {
    return "H";
  }
  else if ( is_pauli( g ) )
  {
    const auto& tag = boost::any_cast<pauli_tag>( g.type() );

    switch ( tag.axis )
    {
    case pauli_axis::X:
      return ( tag.root == 1u ) ? "X" : "⁈";

    case pauli_axis::Y:
      return ( tag.root == 1u ) ? "Y" : "⁈";

    case pauli_axis::Z:
      switch ( tag.root )
      {
      case 1u:
        return "Z";
      case 2u:
        return "S";
      case 4u:
        return tag.adjoint ? "Ŧ" : "T";
      default:
        return "⁈";
      }
    default:
      return "⁈";
    }
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
        auto s = is_stg( *itGate ) ? "■" : ( v.polarity() ? settings.control_char : settings.negative_control_char );
        line_chars[v.line()].push_back( std::make_tuple( gate_index, s ) );
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

    boost::for_each( boost::counting_range( 0u, circ.num_gates() ), [&settings]( unsigned n ) { settings.os << ( n % 10 ); } );
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

void print_circuit_iqc( std::ostream& os, const circuit& circ )
{
  const auto n = circ.lines();

  /* create string */
  std::vector<std::string> circ_str;

  for ( const auto& g : circ )
  {
    std::string gate_str( n, ' ' );

    const auto t = g.targets().front();
    gate_str[t] = 'X';

    for ( const auto& c : g.controls() )
    {
      gate_str[c.line()] = '1' + t;
    }

    circ_str.push_back( gate_str );
  }

  for ( auto l = 0u; l < n; ++l )
  {
    for ( auto g = 0u; g < circ_str.size(); ++g )
    {
      const auto chr = circ_str[g][l];
      if ( chr == ' ' )
      {
        os << " I   ";
      }
      else if ( chr == 'X' )
      {
        os << " X   ";
      }
      else
      {
        os << boost::format( "C(%c) " ) % chr;
      }
    }
    os << std::endl;
  }
}

format_iqc::format_iqc( const circuit& circ )
  : circ( circ ) {}

std::ostream& operator<<( std::ostream& os, const format_iqc& fmt )
{
  print_circuit_iqc( os, fmt.circ );
  return os;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
