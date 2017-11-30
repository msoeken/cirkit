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

#include "xmg_extract.hpp"

#include <map>

#include <boost/format.hpp>

#include <core/properties.hpp>
#include <classical/xmg/xmg_cover.hpp>
#include <classical/xmg/xmg_rewrite.hpp>

namespace cirkit
{

xmg_graph xmg_extract( const xmg_graph& xmg, xmg_node root, const std::vector<xmg_node>& leaves )
{
  auto xmg_copy = xmg;

  /* remove all outputs from xmg_copy and add one to the root node of the cut */
  xmg_copy.outputs().clear();
  xmg_copy.create_po( xmg_function( root, false ), "outroot" );

  xmg_graph xcut;

  prefill_func_t prefill = [leaves]( xmg_graph& xcut, std::map<xmg_node, xmg_function>& old_to_new ) {
    auto i = 0u;
    for ( auto leaf : leaves ) {
      old_to_new[leaf] = xcut.create_pi( boost::str( boost::format( "x%d" ) % i ) );
      ++i;
    }
  };

  auto settings = std::make_shared<properties>();
  settings->set( "prefill", prefill );

  const auto o = xmg_rewrite_top_down_inplace( xcut, xmg_copy, rewrite_default_maj, rewrite_default_xor, {}, settings ).front();
  xcut.create_po( o, "o" );

  return xcut;
}

xmg_graph xmg_extract_lut( const xmg_graph& xmg, xmg_node root )
{
  assert( xmg.has_cover() && xmg.cover().has_cut( root ) );

  const auto& cut = xmg.cover().cut( root );
  std::vector<xmg_node> leaves( std::begin( cut ), std::end( cut ) );

  return xmg_extract( xmg, root, leaves );
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
