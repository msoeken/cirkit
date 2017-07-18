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

#include "stg_map_precomp.hpp"

#include <boost/dynamic_bitset.hpp>

#include <core/utils/timer.hpp>
#include <classical/functions/linear_classification.hpp>
#include <classical/functions/spectral_canonization.hpp>
#include <classical/utils/truth_table_utils.hpp>
#include <reversible/target_tags.hpp>
#include <reversible/synthesis/optimal_quantum_circuits.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

gate& append_stg_from_line_map( circuit& circ, uint64_t func, uint64_t affine_class, const std::vector<unsigned>& line_map )
{
  auto& g = circ.append_gate();

  const auto num_vars = line_map.size() - 1;

  for ( auto i = 0u; i < num_vars; ++i )
  {
    g.add_control( make_var( line_map[i], true ) );
  }
  g.add_target( line_map.back() );

  stg_tag stg;
  stg.function = boost::dynamic_bitset<>( 1 << num_vars, func );
  stg.affine_class = boost::dynamic_bitset<>( 1 << num_vars, affine_class );
  g.set_type( stg );
  circ.annotate( g, "affine", tt_to_hex( stg.affine_class ) );

  return g;
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

void stg_map_precomp( circuit& circ, uint64_t function, unsigned num_vars,
                      const std::vector<unsigned>& line_map,
                      const stg_map_precomp_params& params,
                      stg_map_precomp_stats& stats )
{
  uint64_t cfunc{};

  /* classification */
  increment_timer t( &stats.class_runtime );

  const auto it = stats.class_hash[num_vars - 2u].find( function );
  if ( it == stats.class_hash[num_vars - 2u].end() )
  {
    if ( params.class_method == 0u ) /* spectral */
    {
      const auto idx = get_spectral_class( tt( 1 << num_vars, function ) );
      cfunc = optimal_quantum_circuits::spectral_classification_representative[num_vars - 2u][idx];
    }
    else                             /* affine */
    {
      cfunc = exact_affine_classification_output( function, num_vars );
    }
    stats.class_hash[num_vars - 2u].insert( std::make_pair( function, cfunc ) );
  }
  else
  {
    cfunc = it->second;
  }
  ++stats.class_counter[num_vars - 2u][optimal_quantum_circuits::spectral_classification_index[num_vars - 2u].at( cfunc )];

  append_stg_from_line_map( circ, function, cfunc, line_map );
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
