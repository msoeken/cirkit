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

#include "pidd_debugging.hpp"

#include <cstdlib>
#include <fstream>

#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/format.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/irange.hpp>

#include <core/utils/range_utils.hpp>
#include <core/utils/timer.hpp>

#include <reversible/functions/copy_circuit.hpp>
#include <reversible/io/print_circuit.hpp>
#include <reversible/utils/foreach_gate.hpp>
#include <reversible/utils/permutation.hpp>

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

bool pidd_debugging( const circuit& circ, const binary_truth_table& spec,
                     const properties::ptr& settings,
                     const properties::ptr& statistics )
{
  using boost::format;
  using boost::str;
  using boost::adaptors::transformed;

  /* settings */
  auto with_negated = get( settings, "with_negated", false );

  /* timer */
  properties_timer t( statistics );

  std::ofstream os( "/tmp/pidd.perm", std::ofstream::out );

  // write symbols
  os << "symbol " << any_join( boost::irange( 0u, 1u << circ.lines() ), " " ) << std::endl;

  // write Toffoli gates
  auto gidx = 0u;
  foreach_gate( circ.lines(), with_negated, [&]( const circuit& c ) {
      os << format( "T%d = %s" ) % gidx++ % permutation_to_string( circuit_to_permutation( c ) ) << std::endl;
    });
  auto range = boost::irange( 0u, gidx ) | transformed( []( unsigned i ) { return str( format( "T%d" ) % i ); } );
  os << "TOFS = " << boost::join( range, " + " ) << std::endl;

  // write function permutation
  os << format( "F = %s" ) % permutation_to_string( truth_table_to_permutation( spec ) ) << std::endl;

  // write missing gate permutations for each position
  circuit left, right( circ.lines() );
  copy_circuit( circ, left );

  for ( auto i = 0u; i <= circ.num_gates(); ++i )
  {
    auto pleft  = permutation_to_string( circuit_to_permutation( left ) );
    auto pright = permutation_to_string( circuit_to_permutation( right ) );

    os << format( "C = %s * TOFS * %s" ) % pleft % pright << std::endl
       << "print C & F" << std::endl;

    // next or break
    if ( i == circ.num_gates() ) break;
    auto pos = left.num_gates() - 1u;
    right.prepend_gate() = left[pos];
    left.remove_gate_at( pos );
  }

  os.close();

  auto sresult = system( "perm /tmp/pidd.perm | grep -v -e \"^ 0$\" | wc -l > /tmp/pidd.log" );
  assert( !sresult );

  std::ifstream is( "/tmp/pidd.log", std::ifstream::in );
  std::string line;
  std::getline( is, line );
  boost::trim( line );
  is.close();

  return ( line != "0" );
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
