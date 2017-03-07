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

#include "write_liquid.hpp"

#include <fstream>
#include <vector>

#include <boost/algorithm/string/join.hpp>
#include <boost/format.hpp>

#include <core/utils/string_template.hpp>
#include <reversible/target_tags.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

void write_liquid( const circuit& circ, std::ostream& os, const properties::ptr& settings )
{
  const auto dump_statement = get( settings, "dump_statement", false );

  string_template t(
    "#if INTERACTIVE\n"
    "#r @\"..\\bin\\Liquid1.dll\"\n"
    "#else\n"
    "namespace Microsoft.Research.Liquid\n"
    "#endif\n\n"
    "open System\n\n"
    "open Microsoft.Research.Liquid\n"
    "open Util\n"
    "open Operations\n\n"
    "module Script =\n"
    "    [<LQD>]\n"
    "    let RevKit() =\n"
    "        let MPMCT(qs: Qubits)(negcs: Qubits) =\n"
    "            let nc = qs.Length - 1           // number of controls\n"
    "            let cs = List.replicate nc Cgate // control lines\n"
    "            let cg = List.fold (<<) id cs    // make controlled operation\n"
    "            for n in negcs do X [n]          // negative controls\n"
    "            cg X qs                          // apply gate\n"
    "            for n in negcs do X [n]          // negative controls\n\n"
    "        let ops(qs: Qubits) =\n"
    "{{ gates }}\n"
    "        let ket  = Ket({{ lines }})\n"
    "        let qs   = ket.Qubits\n"
    "        let circ = ( Circuit.Compile ops qs ).GrowGates( ket )\n\n"
    "        show \"#gates in grown circuit: %d\" (circ.GateCount())\n{{ dump }}\n"
    "#if INTERACTIVE\n"
    "do Script.RevKit()\n"
    "#endif\n" );

  std::string gstrs;
  for ( const auto& g : circ )
  {
    assert( is_toffoli( g ) );

    std::string gstr = "          MPMCT [";
    std::vector<std::string> neg;

    for ( const auto& c : g.controls() )
    {
      gstr += boost::str( boost::format( "qs.[%d];" ) % c.line() );

      if ( !c.polarity() )
      {
        neg.push_back( boost::str( boost::format( "qs.[%d]" ) % c.line() ) );
      }
    }
    gstr += boost::str( boost::format( "qs.[%d]] [%s]\n" ) % g.targets().front() % boost::join( neg, ";" ) );

    gstrs += gstr;
  }

  std::string dump;
  if ( dump_statement )
  {
    dump = "        circ.Dump()\n";
  }

  os << t( std::unordered_map<std::string, std::string>( {
        {"gates", gstrs},
        {"lines", std::to_string( circ.lines() )},
        {"dump", dump}
      } ) );
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

void write_liquid( const circuit& circ, const std::string& filename, const properties::ptr& settings )
{
  std::ofstream os( filename.c_str(), std::ofstream::out );
  write_liquid( circ, os, settings );
  os.close();
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
