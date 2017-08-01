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

#include "add_stg.hpp"

#include <classical/functions/spectral_canonization2.hpp>
#include <reversible/target_tags.hpp>
#include <reversible/functions/add_gates.hpp>

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

void add_stg_as_other( circuit& circ, const tt& func, const tt& func_real )
{
  const auto num_vars = tt_num_vars( func );

  std::vector<trans_t> trans;

  spectral_normalization_params params;
  spectral_normalization_stats stats;

  const auto func_norm = spectral_normalization( func, params, stats );
  std::copy( stats.transforms.begin(), stats.transforms.end(), std::back_inserter( trans ) );
  const auto func_real_norm = spectral_normalization( func_real, params, stats );
  std::copy( stats.transforms.rbegin(), stats.transforms.rend(), std::back_inserter( trans ) );

  auto index = 0u;

  for ( const auto& t : trans )
  {
    switch ( t.kind )
    {
    default: assert( false );
    case 1:
      insert_fredkin( circ, index, {}, t.var1, t.var2 );
      insert_fredkin( circ, index, {}, t.var1, t.var2 );
      ++index;
      break;
    case 2:
      insert_not( circ, index, t.var1 );
      insert_not( circ, index, t.var1 );
      ++index;
      break;
    case 3:
      insert_not( circ, index, num_vars );
      break;
    case 4:
      insert_cnot( circ, index, t.var2, t.var1 );
      insert_cnot( circ, index, t.var2, t.var1 );
      ++index;
      break;
    case 5:
      insert_cnot( circ, index, t.var1, num_vars );
      break;
    }
  }

  auto& g = circ.insert_gate( index );
  g.set_type( stg_tag( func_real ) );
  for ( auto i = 0u; i < num_vars; ++i )
  {
    g.add_control( make_var( i, true ) );
  }
  g.add_target( num_vars );
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
