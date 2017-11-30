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

#include "xmg_cofactor.hpp"

#include <classical/xmg/xmg_rewrite.hpp>

namespace cirkit
{

xmg_graph xmg_cofactor( const xmg_graph& xmg, unsigned input_index, bool value )
{
  xmg_graph dest( xmg.name() );
  const auto n = xmg.inputs().size();

  std::vector<xmg_function> pi_mapping( n );

  for ( auto i = 0u; i < n; ++i )
  {
    if ( i == input_index )
    {
      pi_mapping[i] = dest.get_constant( value );
    }
    else
    {
      pi_mapping[i] = dest.create_pi( xmg.inputs()[i].second );
    }
  }

  const auto fs = xmg_rewrite_top_down_inplace( dest, xmg, rewrite_default_maj, rewrite_default_xor, pi_mapping );

  for ( auto i = 0u; i < xmg.outputs().size(); ++i )
  {
    dest.create_po( fs[i], xmg.outputs()[i].second );
  }

  return dest;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
