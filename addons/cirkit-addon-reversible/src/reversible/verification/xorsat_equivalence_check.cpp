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

#include "xorsat_equivalence_check.hpp"

#include <fstream>

#include <boost/format.hpp>
#include <boost/range/algorithm_ext/iota.hpp>

#include <core/utils/system_utils.hpp>
#include <core/utils/timer.hpp>
#include <reversible/target_tags.hpp>
#include <reversible/functions/add_circuit.hpp>
#include <reversible/functions/reverse_circuit.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

circuit create_identity_miter( const circuit& circ1, const circuit& circ2 )
{
  circuit id;

  reverse_circuit( circ1, id );
  append_circuit( id, circ2 );

  return id;
}

void write_to_dimacs( const circuit& circ, const std::string& filename )
{
  using boost::format;
  using boost::str;

  std::vector<unsigned> line_to_var( circ.lines() );
  boost::iota( line_to_var, 1u );

  auto next_var = circ.lines() + 1u;

  std::ofstream os( filename.c_str() );

  for ( const auto& g : circ )
  {
    assert( is_toffoli( g ) );
    const auto target = g.targets().front();

    switch ( g.controls().size() )
    {
    case 0u:
      os << format( "x%d %d 0" ) % line_to_var[target] % next_var << std::endl;
      line_to_var[target] = next_var++;
      break;

    case 1u:
      {
        const auto c = g.controls().front();
        os << format( "x%s%d %d -%d 0" ) % ( c.polarity() ? "" : "-" ) % line_to_var[c.line()] % line_to_var[target] % next_var << std::endl;
        line_to_var[target] = next_var++;
      }
      break;

    default:
      {
        std::string all;
        for ( const auto& c : g.controls() )
        {
          os << format( "%s%d -%d 0" ) % ( c.polarity() ? "" : "-" ) % line_to_var[c.line()] % next_var << std::endl;
          all += str( format( "%s%d " ) % ( c.polarity() ? "-" : "" ) % line_to_var[c.line()] );
        }
        os << format( "%s%d 0" ) % all % next_var << std::endl;
        os << format( "x%d %d -%d 0" ) % next_var % line_to_var[target] % ( next_var + 1 ) << std::endl;
        line_to_var[target] = next_var + 1;
        next_var += 2u;
      }
      break;
    }
  }

  std::string ors;
  for ( auto i = 0u; i < circ.lines(); ++i )
  {
    os << format( "x-%d %d %d 0" ) % ( next_var + i ) % ( i + 1 ) % line_to_var[i] << std::endl;
    ors += str( format( "%d " ) % ( next_var + i ) );
  }
  os << ors << "0" << std::endl;

  os.close();
}

bool solve_identity_miter( const std::string& filename )
{
  const auto res = execute_and_return( ( "cryptominisat4 " + filename ).c_str() );

  if ( res.second.back() == "s UNSATISFIABLE" )
  {
    return true;
  }
  else
  {
    return false;
  }
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

bool xorsat_equivalence_check( const circuit& circ1, const circuit& circ2,
                               const properties::ptr& settings,
                               const properties::ptr& statistics )
{
  /* settings */
  const auto tmpname = get( settings, "tmpname", std::string( "/tmp/test.cnf" ) );

  /* timing */
  properties_timer t( statistics );

  if ( circ1.lines() != circ2.lines() )
  {
    std::cout << "[e] circuits do not have the same number of lines" << std::endl;
    return false;
  }

  const auto id_circ = create_identity_miter( circ1, circ2 );

  write_to_dimacs( id_circ, tmpname );
  return solve_identity_miter( tmpname );
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
