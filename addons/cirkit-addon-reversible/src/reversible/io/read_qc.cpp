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

#include "read_qc.hpp"

#include <unordered_map>
#include <vector>

#include <boost/regex.hpp>

#include <core/utils/range_utils.hpp>
#include <core/utils/string_utils.hpp>
#include <reversible/gate.hpp>
#include <reversible/functions/add_gates.hpp>

namespace cirkit
{

circuit read_qc( const std::string& filename )
{
  circuit circ;
  std::unordered_map<std::string, unsigned> var2line;

  line_parser( filename, {
      {boost::regex( "^\\.v *(.*)$" ), [&circ, &var2line]( const boost::smatch& m ) {
          std::vector<std::string> variables;
          split_string( variables, m[1u], " " );

          circ.set_lines( variables.size() );
          for ( const auto& v : index( variables ) )
          {
            var2line[v.value + "'"] = v.index;
            var2line[v.value] = v.index;
          }
        }},
      {boost::regex( "^t(\\d+) *(.*)$" ), [&circ, &var2line]( const boost::smatch& m ) {
          std::vector<std::string> lines;
          split_string( lines, m[2u], " " );

          assert( boost::lexical_cast<unsigned>( std::string( m[1u] ) ) == lines.size() );

          gate::control_container controls;
          for ( auto i = 0; i < lines.size() - 1; ++i )
          {
            controls.push_back( make_var( var2line[lines[i]], lines[i].back() != '\'' ) );
          }
          append_toffoli( circ, controls, var2line[lines.back()] );
        }}
    } );

  return circ;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
