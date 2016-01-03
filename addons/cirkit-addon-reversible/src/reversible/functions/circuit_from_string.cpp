/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2016  EPFL
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
    assert( lines[0][0] = 't' );
    assert( boost::lexical_cast<unsigned>( lines[0].substr( 1u ) ) == lines.size() - 1u );

    auto& _gate = circ.append_gate();
    _gate.set_type( toffoli_tag() );

    for ( auto i = 1u; i < lines.size() - 1u; ++i )
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

    const auto c = lines.back()[0];

    assert( lines.back().size() == 1u );
    assert( c >= 'a' && c <= 'z' );

    max = std::max( c - 'a', max );
    _gate.add_target( c - 'a' );
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
