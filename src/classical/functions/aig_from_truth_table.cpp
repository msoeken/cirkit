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

#include "aig_from_truth_table.hpp"

#include <vector>

#include <core/utils/bitset_utils.hpp>
#include <classical/abc/utils/abc_run_command.hpp>
#include <classical/abc/functions/gia_to_cirkit.hpp>
#include <classical/utils/aig_utils.hpp>

#include <fmt/format.h>

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

aig_graph aig_from_truth_table_naive( const kitty::dynamic_truth_table& t, const std::vector<aig_function>& leafs )
{
  aig_graph aig;
  aig_initialize( aig );

  const auto n = t.num_vars();

  std::vector<aig_function> inputs;

  if ( leafs.empty() )
  {
    for ( auto i = 1u; i <= n; ++i )
    {
      inputs.push_back( aig_create_pi( aig, fmt::format( "x{}", i ) ) );
    }
  }
  else
  {
    inputs = leafs;
  }

  const auto f = aig_from_truth_table_naive( aig, t, inputs );
  aig_create_po( aig, f, "f" );

  return aig;
}

aig_function aig_from_truth_table_naive( aig_graph& aig, const kitty::dynamic_truth_table& t, const std::vector<aig_function>& leafs )
{
  const auto n = t.num_vars();
  assert( leafs.size() == n );

  std::vector<aig_function> minterms;

  kitty::for_each_one_bit( t, [&]( uint64_t minterm ) {
    std::vector<aig_function> cubes;
    for ( auto i = 0u; i < n; ++i )
    {
      cubes.push_back( leafs[i] ^ !( ( minterm >> i ) & 1 ) );
    }

    minterms.push_back( aig_create_nary_and( aig, cubes ) );
  });

  return aig_create_nary_or( aig, minterms );
}

aig_graph aig_from_truth_table( const kitty::dynamic_truth_table& t )
{
  std::stringstream s;
  kitty::print_hex( t, s );

  return abc_run_command( fmt::format( "read_truth {}; strash; short_names; dc2; &get -n", s.str() ) );
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
