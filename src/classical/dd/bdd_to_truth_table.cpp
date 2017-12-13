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

#include "bdd_to_truth_table.hpp"

#include <iostream>

#include <classical/dd/dd_depth_first.hpp>
#include <core/utils/timer.hpp>

using namespace std::placeholders;

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

kitty::dynamic_truth_table bdd_to_truth_table_dfs( const bdd& b )
{
  auto nvars = b.manager->num_vars();
  std::map<unsigned, kitty::dynamic_truth_table> tt_map = {{0u, kitty::dynamic_truth_table( nvars )},
                                                           {1u, ~kitty::dynamic_truth_table( nvars )}};

  auto f = [&]( const bdd& n ) {
    kitty::dynamic_truth_table xi( nvars );
    kitty::create_nth_var( xi, n.var() );

    tt_map.emplace( std::make_pair( n.index, ( xi & tt_map.at( n.high().index ) ) | ( ~xi & tt_map.at( n.low().index ) ) ) );
  };

  dd_depth_first( b, detail::node_func_t<bdd>( f ) );

  return tt_map.at( b.index );
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

kitty::dynamic_truth_table bdd_to_truth_table( const bdd& b,
                                               const properties::ptr& settings,
                                               const properties::ptr& statistics )
{
  /* settings */
  auto method = get( settings, "method", bdd_to_truth_table_method::dfs );

  /* timing */
  properties_timer t( statistics );

  switch ( method )
  {
  case bdd_to_truth_table_method::dfs:
    return bdd_to_truth_table_dfs( b );
  case bdd_to_truth_table_method::visit:
    std::cerr << "[e] not yet implemented" << std::endl;
    assert( false );
  }

  std::cerr << "[e] invalid method selected" << std::endl;
  assert( false );

  return kitty::dynamic_truth_table( 0 );
}

std::vector<unsigned> bdds_to_truth_table_unsigned( const std::vector<bdd>& fs,
                                                    const properties::ptr& settings,
                                                    const properties::ptr& statistics )
{
  assert( !fs.empty() );

  /* TODO: run-time measurement is wrong */
  std::vector<kitty::dynamic_truth_table> tts;
  std::transform( fs.begin(), fs.end(), std::back_inserter( tts ), std::bind( bdd_to_truth_table, _1, settings, statistics ) );

  std::vector<unsigned> values( tts.front().num_bits() );
  for ( auto i = 0u; i < tts.front().num_bits(); ++i )
  {
    for ( auto j = 0u; j < tts.size(); ++j )
    {
      values[i] |= kitty::get_bit( tts[i], j ) << j;
    }
  }

  return values;
}

std::vector<int> bdds_to_truth_table_signed( const std::vector<bdd>& fs,
                                             const properties::ptr& settings,
                                             const properties::ptr& statistics )
{
  assert( !fs.empty() );

  /* TODO: run-time measurement is wrong */
  std::vector<kitty::dynamic_truth_table> tts;
  std::transform( fs.begin(), fs.end(), std::back_inserter( tts ), std::bind( bdd_to_truth_table, _1, settings, statistics ) );

  std::vector<int> values( tts.front().num_bits() );
  for ( auto i = 0u; i < tts.front().num_bits(); ++i )
  {
    for ( auto j = 0u; j < tts.size(); ++j )
    {
      values[i] |= kitty::get_bit( tts[i], j ) << j;
    }

    if ( values[i] >> ( tts.size() - 1 ) & 1 )
    {
      values[i] = -( ~values[i] + 1 );
    }
  }

  return values;
}
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
