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

#include "circuit_utils.hpp"

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

bool has_fully_controlled_gate( const circuit& circ )
{
  const auto n = circ.lines();
  for ( const auto& g : circ )
  {
    if ( g.controls().size() + g.targets().size() == n )
    {
      return true;
    }
  }
  return false;
}

unsigned number_of_qubits( const circuit& circ )
{
  const auto n = circ.lines();
  return has_fully_controlled_gate( circ ) ? n + 1 : n;
}

bool has_negative_control( const gate& g )
{
  return std::find_if( g.controls().begin(), g.controls().end(),
                       []( const variable& v ) { return !v.polarity(); } ) != g.controls().end();
}

boost::dynamic_bitset<> get_line_mask( const gate& g, unsigned lines )
{
  boost::dynamic_bitset<> mask( lines );
  std::for_each( std::begin( g.controls() ), std::end( g.controls() ),
                 [&mask]( const variable& v ) { mask.set( v.line() ); } );
  std::for_each( std::begin( g.targets() ), std::end( g.targets() ),
                 [&mask]( unsigned t ) { mask.set( t ); } );
  return mask;
}

std::vector<unsigned> get_line_map( const gate& g )
{
  std::vector<unsigned> line;
  std::for_each( std::begin( g.controls() ), std::end( g.controls() ),
                 [&line]( const variable& v ) { line.push_back( v.line() ); } );
  std::for_each( std::begin( g.targets() ), std::end( g.targets() ),
                 [&line]( unsigned t ) { line.push_back( t ); } );
  return line;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
