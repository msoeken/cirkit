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

#include <boost/format.hpp>

#include <core/utils/bitset_utils.hpp>
#include <classical/abc/utils/abc_run_command.hpp>
#include <classical/abc/functions/gia_to_cirkit.hpp>
#include <classical/utils/aig_utils.hpp>

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

aig_graph aig_from_truth_table_naive( const tt& t, const std::vector<aig_function>& leafs )
{
  aig_graph aig;
  aig_initialize( aig );

  const auto n = tt_num_vars( t );

  std::vector<aig_function> inputs;

  if ( leafs.empty() )
  {
    for ( auto i = 1u; i <= n; ++i )
    {
      inputs.push_back( aig_create_pi( aig, boost::str( boost::format( "x%d" ) % i ) ) );
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

aig_function aig_from_truth_table_naive( aig_graph& aig, const tt& t, const std::vector<aig_function>& leafs )
{
  const auto n = tt_num_vars( t );
  assert( leafs.size() == n );

  std::vector<aig_function> minterms;

  foreach_minterm( t, [&]( const boost::dynamic_bitset<>& minterm ) {
      std::vector<aig_function> cube;
      for ( auto i = 0u; i < n; ++i )
      {
        cube.push_back( leafs[i] ^ !minterm[i] );
      }

      minterms.push_back( aig_create_nary_and( aig, cube ) );
    } );

  return aig_create_nary_or( aig, minterms );
}

aig_graph aig_from_truth_table( const tt& t )
{
  return abc_run_command( boost::str( boost::format( "read_truth %s; strash; short_names; dc2; &get -n" ) % tt_to_hex( t )  ) );
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
