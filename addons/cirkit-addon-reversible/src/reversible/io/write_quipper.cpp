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

#include "write_quipper.hpp"

#include <fstream>
#include <vector>

#include <boost/format.hpp>

#include <core/utils/range_utils.hpp>
#include <reversible/target_tags.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

void write_quipper( const circuit& circ, std::ostream& os )
{
  std::vector<std::string> line_names;
  std::string              inputs, inputs_sig;
  std::string              constants;
  std::string              outputs, outputs_sig;

  auto pi_ctr = 0u;
  auto c_ctr  = 0u;

  /* prepare */
  for ( auto i = 0u; i < circ.lines(); ++i )
  {
    if ( (bool)circ.constants()[i] )
    {
      line_names.push_back( boost::str( boost::format( "c%d" ) % ++c_ctr ) );
      constants += boost::str( boost::format( "  %s <- qinit %s\n" ) % line_names.back() % ( *circ.constants()[i] ? "True" : "False" ) );
    }
    else
    {
      line_names.push_back( boost::str( boost::format( "x%d" ) % ++pi_ctr ) );

      if ( pi_ctr != 1u )
      {
        inputs += ", ";
        inputs_sig += ", ";
      }
      inputs += line_names.back();
      inputs_sig += "Qubit";
    }

    if ( !circ.garbage()[i] )
    {
      if ( !outputs.empty() )
      {
        outputs += ", ";
        outputs_sig += ", ";
      }
      outputs += line_names.back();
      outputs_sig += "Qubit";
    }
  }

  /* header */
  os << "-- This output has been generated using RevKit" << std::endl
     << "-- ===========================================" << std::endl << std::endl
     << "import Quipper" << std::endl << std::endl
     << boost::format( "revkit_circuit :: (%s) -> Circ (%s)" ) % inputs_sig % outputs_sig << std::endl
     << boost::format( "revkit_circuit (%s) = do" ) % inputs << std::endl
     << constants;

  /* gates */
  for ( const auto& g : circ )
  {
    assert( is_toffoli( g ) );

    std::string controls;

    switch ( g.controls().size() )
    {
    case 0u:
      break;
    case 1u:
      {
        const auto& c = g.controls().front();
        controls = boost::str( boost::format( " `controlled` %s .==. %d" ) % line_names[c.line()] % ( c.polarity() ? 1 : 0 ) );
      } break;
    default:
      {
        std::string cs, ps;
        for ( const auto& c : index( g.controls() ) )
        {
          if ( c.index > 0u )
          {
            cs += ", ";
            ps += ", ";
          }
          cs += line_names[c.value.line()];
          ps += c.value.polarity() ? "1" : "0";
        }
        controls = boost::str( boost::format( " `controlled` [%s] .==. [%s]" ) % cs % ps );
      } break;
    }

    os << boost::format( "  qnot_at %s%s" ) % line_names[g.targets().front()] % controls << std::endl;
  }

  /* output */
  os << boost::format( "  return (%s)" ) % outputs << std::endl << std::endl
     << "main =" << std::endl
     << "  print_simple ASCII revkit_circuit" << std::endl;
}

void write_quipper_ascii( const circuit& circ, std::ostream& os )
{
  std::string              inputs;
  std::string              constants;
  std::string              outputs;

  // auto pi_ctr = 0u;
  // auto c_ctr  = 0u;

  /* prepare */
  for ( auto i = 0u; i < circ.lines(); ++i )
  {
    if ( (bool)circ.constants()[i] )
    {
      constants += boost::str( boost::format( "QInit%d(%d)\n" ) % ( *circ.constants()[i] ? 1 : 0 ) % i );
    }
    else
    {
      if ( !inputs.empty() )
      {
        inputs += ", ";
      }
      inputs += boost::str( boost::format( "%d:Qbit" ) % i );
    }

    if ( !circ.garbage()[i] )
    {
      if ( !outputs.empty() )
      {
        outputs += ", ";
      }
      outputs += boost::str( boost::format( "%d:Qbit" ) % i );
    }
  }

  /* header */
  os << "# This output has been generated using RevKit" << std::endl
     << "# ===========================================" << std::endl << std::endl
     << "Inputs: " << inputs << std::endl
     << constants;

  /* gates */
  for ( const auto& g : circ )
  {
    assert( is_toffoli( g ) );

    std::string controls;

    os << boost::format( "QGate[\"not\"](%d)" ) % g.targets().front();

    if ( !g.controls().empty() )
    {
      os << " with controls=[";

      for ( const auto& c : index( g.controls() ) )
      {
        if ( c.index > 0u )
        {
          os << ",";
        }
        os << boost::format( "%c%d" ) % ( c.value.polarity() ? '+' : '-' ) % c.value.line();
      }

      os << "]";
    }
    os << std::endl;
  }

  /* output */
  os << "Outputs: " << outputs << std::endl;
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

void write_quipper( const circuit& circ, const std::string& filename )
{
  std::ofstream os( filename.c_str(), std::ostream::out );
  write_quipper( circ, os );
}

void write_quipper_ascii( const circuit& circ, const std::string& filename )
{
  std::ofstream os( filename.c_str(), std::ostream::out );
  write_quipper_ascii( circ, os );
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
