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

#include "xmg_dont_cares.hpp"

#include <iostream>
#include <vector>

#include <classical/xmg/xmg_rewrite.hpp>
#include <classical/xmg/xmg_simulate.hpp>
#include <classical/xmg/xmg_utils.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

/* creates a miter in which node is inverted in one of the versions */
xmg_graph xmg_dont_care_miter( const xmg_graph& xmg, xmg_node node )
{
  /* create version with negative node */
  const auto settings = std::make_shared<properties>();
  xmg_substitutes_map_t::value_type map;
  map[node] = xmg_function( node, true );
  settings->set( "substitutes", xmg_substitutes_map_t( map ) );
  const auto xmg_dub = xmg_rewrite_top_down( xmg, rewrite_default_maj, rewrite_default_xor, settings );

  /* create miter and connect outputs */
  auto miter = xmg_merge( xmg, xmg_dub );
  const auto num_outputs = xmg.outputs().size();
  std::vector<xmg_function> xors( num_outputs );
  for ( auto i = 0u; i < num_outputs; ++i )
  {
    xors[i] = miter.create_xor( miter.outputs()[i].first, miter.outputs()[num_outputs + i].first );
  }
  miter.outputs().clear();
  miter.create_po( miter.create_nary_or( xors ), "miter" );

  return miter;
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

bool xmg_is_observable_at_node( const xmg_graph& xmg, xmg_node node, const boost::dynamic_bitset<>& pattern )
{
  const auto miter = xmg_dont_care_miter( xmg, node );

  xmg_pattern_simulator sim( pattern );
  return simulate_xmg_function( miter, miter.outputs().front().first, sim );
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
