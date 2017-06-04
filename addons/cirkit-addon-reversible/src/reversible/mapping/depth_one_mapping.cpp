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

#include "depth_one_mapping.hpp"

#include <iostream>

#include <boost/format.hpp>

#include <core/utils/bitset_utils.hpp>
#include <core/utils/range_utils.hpp>
#include <core/utils/timer.hpp>
#include <reversible/pauli_tags.hpp>
#include <reversible/functions/add_gates.hpp>
#include <reversible/functions/add_line_to_circuit.hpp>

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

circuit depth_one_synthesis( const tt& function, const properties::ptr& settings, const properties::ptr& statistics )
{
  /* settings */

  /* timer */
  properties_timer t( statistics );

  auto n = tt_num_vars( function );

  /* extend function by AND-ing it with next MSB variable */
  auto f_ext = function;
  tt_extend( f_ext, n + 1 );
  auto fv = tt_nth_var( n );
  if ( n < 5u )
  {
    tt_shrink( fv, n + 1 );
  }
  f_ext &= fv;
  ++n;

  /* add lines to circuit and CNOT gates (naive) */
  circuit circ;
  boost::dynamic_bitset<> bs( n, 1u );
  auto idx = 0u;
  do {
    if ( bs.count() == 1 )
    {
      const auto i = bs.find_first();
      add_line_to_circuit( circ, boost::str( boost::format( "x%d" ) % i ), boost::str( boost::format( "y%d" ) % i ) );
    }
    else
    {
      const auto tgt = circ.lines();
      add_line_to_circuit( circ, "0", "0", false, true );

      foreach_bit( bs, [&circ, &idx, tgt]( unsigned ctrl ) {
          insert_cnot( circ, idx, ( 1 << ctrl ) - 1, tgt );
          insert_cnot( circ, idx, ( 1 << ctrl ) - 1, tgt );
          ++idx;
        } );
    }

    inc( bs );
  } while ( bs.any() );

  /* compute spectra */
  const auto spectra = walsh_spectrum( f_ext );
  std::cout << any_join( spectra, " " ) << std::endl;

  for ( auto i = 0u; i < circ.lines(); ++i )
  {
    boost::dynamic_bitset<> decomp( n, -spectra[i + 1] % ( 1 << n ) );

    foreach_bit( decomp, [&circ, &idx, i, n]( unsigned pos ) {
        const auto root = 1 << ( n - 1 - pos );
        insert_pauli( circ, idx++, i, pauli_axis::Z, root );
      } );
  }

  /* Hadamards */
  const auto tgt = ( 1 << ( n - 1 ) ) - 1;
  prepend_hadamard( circ, tgt );
  append_hadamard( circ, tgt );

  return circ;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
