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

#include <core/utils/bitset_utils.hpp>
#include <core/utils/timer.hpp>
#include <classical/dd/dd_depth_first.hpp>

using namespace std::placeholders;

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

tt bdd_to_truth_table_dfs( const bdd& b )
{
  auto nvars = b.manager->num_vars();
  std::map<unsigned, tt> tt_map = { { 0u, tt_const0() },
                                    { 1u, tt_const1() } };
  tt_extend( tt_map[0u], nvars );
  tt_extend( tt_map[1u], nvars );

  auto f = [&]( const bdd& n ) {
    auto xi = tt_nth_var( n.var() );
    tt_extend( xi, nvars );

    tt_map[n.index] = ( xi & tt_map[n.high().index] ) | ( ~xi & tt_map[n.low().index] );
  };

  dd_depth_first( b, detail::node_func_t<bdd>( f ) );

  return tt_map[b.index];
}

int bitset_to_signed( const boost::dynamic_bitset<>& b )
{
  assert( b.size() );

  if ( b.test( b.size() - 1u ) )
  {
    auto bn = ~b;
    return -( inc( bn ).to_ulong() );
  }
  else
  {
    return b.to_ulong();
  }
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

tt bdd_to_truth_table( const bdd& b,
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

  return tt();
}

std::vector<unsigned> bdds_to_truth_table_unsigned( const std::vector<bdd>& fs,
                                                    const properties::ptr& settings,
                                                    const properties::ptr& statistics )
{
  assert( !fs.empty() );

  /* TODO: run-time measurement is wrong */
  std::vector<tt> tts( fs.size() );
  boost::transform( fs, tts.begin(), std::bind( bdd_to_truth_table, _1, settings, statistics ) );

  auto tts_t = transpose( tts );
  std::vector<unsigned> result( tts_t.size() );
  boost::transform( tts_t, result.begin(), std::bind( &boost::dynamic_bitset<>::to_ulong, _1 ) );

  return result;
}

std::vector<int> bdds_to_truth_table_signed( const std::vector<bdd>& fs,
                                             const properties::ptr& settings,
                                             const properties::ptr& statistics )
{
  assert( !fs.empty() );

  /* TODO: run-time measurement is wrong */
  std::vector<tt> tts( fs.size() );
  boost::transform( fs, tts.begin(), std::bind( bdd_to_truth_table, _1, settings, statistics ) );

  auto tts_t = transpose( tts );
  std::vector<int> result( tts_t.size() );
  boost::transform( tts_t, result.begin(), std::bind( bitset_to_signed, _1 ) );

  return result;
}


}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
