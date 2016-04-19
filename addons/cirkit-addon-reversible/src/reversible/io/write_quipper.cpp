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
     << "  print_simple Preview revkit_circuit" << std::endl;
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

void write_quipper( const circuit& circ, const std::string& filename )
{
  std::ofstream os( filename.c_str(), std::ostream::out );
  write_quipper( circ, os );
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
