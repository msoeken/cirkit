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

#include "npn_circuit.hpp"

#include <deque>
#include <vector>

#include <boost/format.hpp>

#include <core/utils/range_utils.hpp>
#include <classical/utils/truth_table_utils.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

void permute_configuration( std::vector<unsigned>& v, unsigned n, unsigned pos )
{
  const auto dist = 1 << pos;             // swap distance
  const auto freq = 1 << ( n - pos - 2 ); // how many groups are swapped

  for ( auto i = 0; i < freq; ++i )
  {
    for ( auto j = 0; j < dist; ++j )
    {
      const auto offset = i * ( 1 << ( pos + 2 ) ) + ( 1 << pos ) + j;
      std::swap( v[offset], v[offset + dist] );
    }
  }
}

void flip_configuration( std::vector<unsigned>& v, unsigned n, unsigned pos )
{
  const auto dist = 1 << pos;             // swap distance
  const auto freq = 1 << ( n - pos - 1 ); // how many groups are swapped

  for ( auto i = 0; i < freq; ++i )
  {
    for ( auto j = 0; j < dist; ++j )
    {
      const auto offset = i * ( 1 << ( pos + 1 ) ) + j;
      std::swap( v[offset], v[offset + dist] );
    }
  }
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

void generate_npn_circuit( std::ostream& os, unsigned n,
                           const properties::ptr& settings,
                           const properties::ptr& statistics )
{
  const auto N = 1 << n;
  std::vector<unsigned> perm( N ); // for different permutations
  std::vector<std::vector<std::string>> wires_computation;
  std::vector<std::string> wires;
  std::vector<std::string> inst;
  std::vector<std::string> assign;

  /* add NPN module to design */
  const auto add_module = [&]() {
    const std::string suffix = any_join( perm, "_" );

    os << boost::format( "module npn_%s( F, Fo );" ) % suffix << std::endl
       << boost::format( "  input  [%d:0] F;" ) % ( N - 1 ) << std::endl
       << boost::format( "  output [%d:0] Fo;" ) % ( N - 1 ) << std::endl;

    for ( auto i = 0u; i < perm.size(); ++i )
    {
      os << boost::format( "  assign Fo[%d] = F[%d];" ) % i % perm[i] << std::endl;
    }

    os << "endmodule" << std::endl << std::endl;

    /* instances */
    inst.push_back( boost::str( boost::format( "  npn_%1% inst_%1%_pos(  F, F_%1%_pos );\n" ) % suffix ) );
    inst.push_back( boost::str( boost::format( "  npn_%1% inst_%1%_neg( ~F, F_%1%_neg );\n" ) % suffix ) );

    /* wires */
    wires.push_back( boost::str( boost::format( "F_%1%_pos" ) % suffix ) );
    wires.push_back( boost::str( boost::format( "F_%1%_neg" ) % suffix ) );
  };

  wires_computation.push_back( wires );   // save the first wires computation before doing <=

  for ( auto i = 0; i < N; ++i )
  {
    perm[i] = i;
  }

  add_module();

  const auto& swap_array = tt_store::i().swaps( n );
  const auto& flip_array = tt_store::i().flips( n );
  const auto total_swaps = swap_array.size();
  const auto total_flips = flip_array.size();

  for ( int i = total_swaps - 1; i >= 0; --i )
  {
    permute_configuration( perm, n, swap_array[i] );
    add_module();
  }

  for ( int j = total_flips - 1; j >= 0; --j )
  {
    permute_configuration( perm, n, 0u );
    flip_configuration( perm, n, flip_array[j] );
    add_module();

    for ( int i = total_swaps - 1; i >= 0; --i )
    {
      permute_configuration( perm, n, swap_array[i] );
      add_module();
    }
  }

  /* add comparators */
  std::deque<std::string> deque( wires.begin(), wires.end() );

  auto id = 0;
  while ( deque.size() > 1 )
  {
    const auto new_wire = boost::str( boost::format( "w%d" ) % id++ );
    const auto w1 = deque.front(); deque.pop_front();
    const auto w2 = deque.front(); deque.pop_front();

    assign.push_back( boost::str( boost::format( "  assign %1% = %2% < %3% ? %2% : %3%;\n" ) % new_wire % w1 % w2 ) );
    wires.push_back( new_wire );
    deque.push_back( new_wire );
  }

  /* assign the output */
  assign.push_back( boost::str( boost::format( "  assign Fr = %s;\n" ) % deque.front() ) );

  os << "module npn_circuit( F, Fr );" << std::endl
     << boost::format( "  input  [%d:0] F;" ) % ( N - 1 ) << std::endl
     << boost::format( "  output [%d:0] Fr;" ) % ( N - 1 ) << std::endl
     << boost::format( "  wire   [%d:0] %s;" ) % ( N - 1 ) % boost::join( wires, ", " )
     << boost::join( inst, "" ) << std::endl
     << boost::join( assign, "" ) << std::endl
     << "endmodule" << std::endl;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
