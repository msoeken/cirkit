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

#include "xorsat_equivalence_check.hpp"

#include <fstream>
#include <string>
#include <vector>

#include <boost/format.hpp>
#include <boost/range/algorithm_ext/iota.hpp>

#include <core/utils/range_utils.hpp>
#include <core/utils/system_utils.hpp>
#include <core/utils/timer.hpp>
#include <reversible/target_tags.hpp>
#include <reversible/functions/add_circuit.hpp>
#include <reversible/functions/add_gates.hpp>
#include <reversible/functions/copy_circuit.hpp>
#include <reversible/functions/reverse_circuit.hpp>
#include <reversible/utils/permutation.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

permutation_t derive_output_permutation( const std::vector<std::string>& a, const std::vector<std::string>& b )
{
  permutation_t perm( a.size() );

  for ( auto i = 0u; i < a.size(); ++i )
  {
    const auto it = std::find( a.begin(), a.end(), b[i] );

    if ( it == a.end() )
    {
      throw boost::str( boost::format( "cannot find output %s in a" ) % b[i] );
    }

    perm[i] = std::distance( a.begin(), it );
  }

  return perm;
}

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
  const auto res = execute_and_return( ( "cryptominisat5 " + filename ).c_str() );

  if ( res.second.back() == "s UNSATISFIABLE" )
  {
    return true;
  }
  else
  {
    std::cout << res.second.back() << std::endl;
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
  const auto name_mapping = get( settings, "name_mapping", false );
  const auto tmpname      = get( settings, "tmpname", std::string( "/tmp/test.cnf" ) );

  /* timing */
  properties_timer t( statistics );

  if ( circ1.lines() != circ2.lines() )
  {
    std::cout << "[e] circuits do not have the same number of lines" << std::endl;
    return false;
  }

  circuit id_circ;

  if ( name_mapping )
  {
    circuit circ2_copy;
    copy_circuit( circ2, circ2_copy );

    const auto operm = derive_output_permutation( circ1.outputs(), circ2.outputs() );

    //std::cout << "operm: " << any_join( operm, " " ) << std::endl;

    const auto idx = circ2_copy.num_gates();
    for ( const auto& t : permutation_to_transpositions( operm ) )
    {
      //std::cout << t.first << " " << t.second << std::endl;
      insert_cnot( circ2_copy, idx, t.first, t.second );
      insert_cnot( circ2_copy, idx, t.second, t.first );
      insert_cnot( circ2_copy, idx, t.first, t.second );
    }

    id_circ = create_identity_miter( circ1, circ2_copy );
  }
  else
  {
    id_circ = create_identity_miter( circ1, circ2 );
  }

  write_to_dimacs( id_circ, tmpname );
  return solve_identity_miter( tmpname );
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
