/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2016  EPFL
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
#include <vector>

#include <boost/lexical_cast.hpp>

#include <core/utils/string_utils.hpp>

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

    assert( lines.size() > 1u );
    assert( lines[0][0] == 't' || lines[0][0] == 'f' );
    assert( boost::lexical_cast<unsigned>( lines[0].substr( 1u ) ) == lines.size() - 1u );

    auto& _gate = circ.append_gate();

    auto num_targets = 1u;
    switch ( lines[0][0] )
    {
    case 't':
      _gate.set_type( toffoli_tag() );
      break;
    case 'f':
      num_targets = 2u;
      _gate.set_type( fredkin_tag() );
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
      assert( lines[i].size() == 1u );

      const auto c = lines[i][0];
      assert( c >= 'a' && c <= 'z' );

      max = std::max( c - 'a', max );
      _gate.add_target( c - 'a' );
    }
  }

  circ.set_lines( max + 1u );

  return circ;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
