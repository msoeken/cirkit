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

#include "truth_table_from_bitset.hpp"

#include <classical/utils/truth_table_utils.hpp>

namespace cirkit
{

binary_truth_table truth_table_from_bitset_direct( const boost::dynamic_bitset<>& bs )
{
  const auto bw = tt_num_vars( bs );

  binary_truth_table spec;

  for ( auto i = 0u; i < ( 1u << bw ); ++i )
  {
    auto in = number_to_truth_table_cube( i, bw );
    binary_truth_table::cube_type out( 1u, boost::optional<bool>( bs.test( i ) ) );
    spec.add_entry( in, out );
  }

  return spec;
}

binary_truth_table truth_table_from_bitset( const boost::dynamic_bitset<>& bs )
{
  const auto num_vars = tt_num_vars( bs );
  const auto bw = ( ( bs.count() << 1u ) == bs.size() ) ? num_vars : num_vars + 1u;

  binary_truth_table spec;

  for ( auto i = 0u; i < ( 1u << bw ); ++i )
  {
    auto in = number_to_truth_table_cube( i, bw );
    binary_truth_table::cube_type out( bw, boost::optional<bool>() );
    if ( i < bs.size() )
    {
      out[0u] = bs[i];
    }
    spec.add_entry( in, out );
  }

  return spec;
}

binary_truth_table truth_table_from_bitset_bennett( const boost::dynamic_bitset<>& bs )
{
  const auto num_vars = tt_num_vars( bs );
  const auto bw = 1u << num_vars;
  binary_truth_table spec;

  for ( auto c = 0; c < 2; ++c )
  {
    for ( auto i = 0u; i < bw; ++i )
    {
      auto in = number_to_truth_table_cube( i, num_vars );
      auto out = number_to_truth_table_cube( i, num_vars );
      in.push_back( c );
      out.push_back( bs[i] != c );
      spec.add_entry( in, out );
    }
  }

  return spec;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
