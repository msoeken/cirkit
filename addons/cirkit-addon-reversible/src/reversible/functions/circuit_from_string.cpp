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

#include "circuit_from_string.hpp"

#include <algorithm>
#include <sstream>
#include <vector>

#include <boost/dynamic_bitset.hpp>
#include <boost/lexical_cast.hpp>

#include <core/utils/conversion_utils.hpp>
#include <core/utils/string_utils.hpp>

#include <reversible/pauli_tags.hpp>
#include <reversible/target_tags.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

circuit circuit_from_string( const std::string& description, const std::string& sep )
{
  circuit circ;

  std::vector<std::string> gates;
  split_string( gates, description, sep );

  int max = 0;

  for ( const auto& g : gates )
  {
    std::vector<std::string> lines;
    split_string( lines, g, " " );

    if ( lines.size() <= 1u )
    {
      std::cout << "[e] cannot understand " << g << std::endl;
      assert( false );
    }

    const auto tc = lines[0][0]; /* target char */
    assert( tc == 't' || tc == 'f' || tc == 's' || tc == 'X' || tc == 'Y' || tc == 'Z' || tc == 'H' );

    /* single target gate comes with function */
    boost::dynamic_bitset<> func;
    if ( tc == 's' )
    {
      assert( lines[0].size() > 3u );
      const auto fstr = lines[0].substr( 2u, lines[0].size() - 3u );
      func = boost::dynamic_bitset<>( convert_hex2bin( fstr ) );
    }

    /* check if there is a root */
    bool adjoint = false;
    auto adj_offset = 0u;
    if ( ( tc == 'X' || tc == 'Y' || tc == 'Z' ) && lines[0].size() >= 2 && lines[0][1] == '+' )
    {
      adjoint = true;
      adj_offset = 1u;
    }

    auto root = 1u;
    if ( ( tc == 'X' || tc == 'Y' || tc == 'Z' ) && lines[0].size() >= 4 + adj_offset && lines[0][1 + adj_offset] == '[' && lines[0].back() == ']' )
    {
      root = boost::lexical_cast<unsigned>( lines[0].substr( 2 + adj_offset, lines[0].size() - 3u - adj_offset ) );
    }

    auto& _gate = circ.append_gate();

    auto num_targets = 1u;
    switch ( tc )
    {
    case 't':
      _gate.set_type( toffoli_tag() );
      break;
    case 'f':
      num_targets = 2u;
      _gate.set_type( fredkin_tag() );
      break;
    case 's':
      _gate.set_type( stg_tag( func ) );
      break;
    case 'X':
      num_targets = 1u;
      _gate.set_type( pauli_tag( pauli_axis::X, root, adjoint ) );
      break;
    case 'Y':
      num_targets = 1u;
      _gate.set_type( pauli_tag( pauli_axis::Y, root, adjoint ) );
      break;
    case 'Z':
      num_targets = 1u;
      _gate.set_type( pauli_tag( pauli_axis::Z, root, adjoint ) );
      break;
    case 'H':
      num_targets = 1u;
      _gate.set_type( hadamard_tag() );
      break;
    }

    for ( auto i = 1u; i < lines.size() - num_targets; ++i )
    {
      char c;
      bool p;

      if ( lines[i][0] == '-' )
      {
        assert( lines[i].size() == 2u );
        c = lines[i][1];
        p = false;
      }
      else
      {
        assert( lines[i].size() == 1u );
        c = lines[i][0];
        p = true;
      }

      assert( c >= 'a' && c <= 'z' );

      max = std::max( c - 'a', max );
      _gate.add_control( make_var( c - 'a', p ) );
    }

    for ( auto i = lines.size() - num_targets; i < lines.size(); ++i )
    {
      if ( lines[i].size() != 1u )
      {
        std::cout << "[e] problem parsing target " << lines[i] << std::endl;
        assert( false );
      }

      const auto c = lines[i][0];
      assert( c >= 'a' && c <= 'z' );

      max = std::max( c - 'a', max );
      _gate.add_target( c - 'a' );
    }
  }

  circ.set_lines( max + 1u );

  return circ;
}

std::string circuit_to_string( const circuit& circ, const std::string& sep )
{
  std::stringstream str;
  auto first = true;

  for ( const auto& g : circ )
  {
    if ( first )
    {
      first = false;
    }
    else
    {
      str << sep;
    }

    if ( is_toffoli( g ) )
    {
      str << "t" << ( g.controls().size() + 1u );

      for ( const auto& c : g.controls() )
      {
        str << " " << ( c.polarity() ? "" : "-" ) << std::string( 1, 'a' + c.line() );
      }

      str << " " << std::string( 1, 'a' + g.targets().front() );
    }
    else if ( is_fredkin( g ) )
    {
      str << "f" << ( g.controls().size() + 2u );

      for ( const auto& c : g.controls() )
      {
        str << " " << ( c.polarity() ? "" : "-" ) << std::string( 1, 'a' + c.line() );
      }

      str << " " << std::string( 1, 'a' + g.targets()[0u] ) << " " << std::string( 1, 'a' + g.targets()[1u] );
    }
    else if ( is_pauli( g ) )
    {
      const auto& tag = boost::any_cast<pauli_tag>( g.type() );
      switch ( tag.axis )
      {
      case pauli_axis::X: str << "X"; break;
      case pauli_axis::Y: str << "Y"; break;
      case pauli_axis::Z: str << "Z"; break;
      }

      if ( tag.adjoint )
      {
        str << "+";
      }

      if ( tag.root != 1u )
      {
        str << "[" << tag.root << "]";
      }

      str << " " << std::string( 1, 'a' + g.targets().front() );
    }
    else if ( is_hadamard( g ) )
    {
      str << "H " << std::string( 1, 'a' + g.targets().front() );
    }
    else
    {
      assert( false );
    }
  }

  return str.str();
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
