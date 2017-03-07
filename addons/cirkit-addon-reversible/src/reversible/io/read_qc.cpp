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

#include "read_qc.hpp"

#include <cmath>
#include <unordered_map>
#include <vector>

#include <boost/format.hpp>

#include <core/utils/range_utils.hpp>
#include <core/utils/string_utils.hpp>
#include <reversible/gate.hpp>
#include <reversible/pauli_tags.hpp>
#include <reversible/functions/add_gates.hpp>

namespace cirkit
{

circuit read_qc( const std::string& filename )
{
  circuit circ;
  std::unordered_map<std::string, unsigned> var2line;

  line_parser( filename, {
      {std::regex( "^ *#.*$" ), []( const std::smatch& m ) {
          /* comment */
        }},
      {std::regex("^ *$" ), []( const std::smatch& m ) {
          /* empty line */
        }},
      {std::regex( "^\\.v +(.*)$" ), [&circ, &var2line]( const std::smatch& m ) {
          std::vector<std::string> variables;
          split_string( variables, m[1u], " " );

          circ.set_lines( variables.size() );
          for ( const auto& v : index( variables ) )
          {
            var2line[v.value + "'"] = v.index;
            var2line[v.value] = v.index;
          }
        }},
      {std::regex( "^\\.i +(.*)$" ), [&circ, &var2line]( const std::smatch& m ) {
          std::vector<std::string> inputs( circ.lines(), "0" );
          std::vector<constant> constants( circ.lines(), constant( false ) );

          std::vector<std::string> vins;
          split_string( vins, m[1u], " " );

          const auto fmt_str = boost::str( boost::format( "pi%%0%dd" ) % ( floor( log10( vins.size() ) ) + 1 ) );

          for ( const auto& i : index( vins ) )
          {
            inputs[var2line[i.value]] = boost::str( boost::format( fmt_str ) % i.index );
            constants[var2line[i.value]] = constant();
          }

          circ.set_inputs( inputs );
          circ.set_constants( constants );
        }},
      {std::regex( "^\\.o +(.*)$" ), [&circ, &var2line]( const std::smatch& m ) {
          std::vector<std::string> outputs( circ.lines(), "--" );
          std::vector<bool> garbage( circ.lines(), true );

          std::vector<std::string> vouts;
          split_string( vouts, m[1u], " " );

          const auto fmt_str = boost::str( boost::format( "po%%0%dd" ) % ( floor( log10( vouts.size() ) ) + 1 ) );

          for ( const auto& i : index( vouts ) )
          {
            outputs[var2line[i.value]] = boost::str( boost::format( fmt_str ) % i.index );
            garbage[var2line[i.value]] = false;
          }

          circ.set_outputs( outputs );
          circ.set_garbage( garbage );
        }},
      {std::regex( "BEGIN|END" ), []( const std::smatch& m ) {
        }},
      {std::regex( "^t(of|\\d+) *(.*)$" ), [&circ, &var2line]( const std::smatch& m ) {
          std::vector<std::string> lines;
          split_string( lines, m[2u], " " );

          if ( std::string( m[1u] ) != "of" )
          {
            assert( boost::lexical_cast<unsigned>( std::string( m[1u] ) ) == lines.size() );
          }

          gate::control_container controls;
          for ( auto i = 0u; i < lines.size() - 1; ++i )
          {
            controls.push_back( make_var( var2line[lines[i]], lines[i].back() != '\'' ) );
          }
          append_toffoli( circ, controls, var2line[lines.back()] );
        }},
      {std::regex( "^H *(.*)$" ), [&circ, &var2line]( const std::smatch& m ) {
          std::vector<std::string> lines;
          split_string( lines, m[1u], " " );

          assert( lines.size() == 1u );

          append_hadamard( circ, var2line[lines.back()] );
        }},
      {std::regex( "^X *(.*)$" ), [&circ, &var2line]( const std::smatch& m ) {
          std::vector<std::string> lines;
          split_string( lines, m[1u], " " );

          assert( lines.size() == 1u );

          append_not( circ, var2line[lines.back()] );
        }},
      {std::regex( "^Z *(.*)$" ), [&circ, &var2line]( const std::smatch& m ) {
          std::vector<std::string> lines;
          split_string( lines, m[1u], " " );

          assert( lines.size() == 1u );

          append_pauli( circ, var2line[lines.back()], pauli_axis::Z );
        }},
      {std::regex( "^(S|P)(\\*?) *(.*)$" ), [&circ, &var2line]( const std::smatch& m ) {
          std::vector<std::string> lines;
          split_string( lines, m[3u], " " );

          assert( lines.size() == 1u );

          append_pauli( circ, var2line[lines.back()], pauli_axis::Z, 2u, !std::string( m[2u] ).empty() );
        }},
      {std::regex( "^T(\\*?) *(.*)$" ), [&circ, &var2line]( const std::smatch& m ) {
          std::vector<std::string> lines;
          split_string( lines, m[2u], " " );

          assert( lines.size() == 1u );

          append_pauli( circ, var2line[lines.back()], pauli_axis::Z, 4u, !std::string( m[1u] ).empty() );
        }}
    }, false );

  return circ;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
