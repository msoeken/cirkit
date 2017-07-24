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

/**
 * @file lhrs_params.hpp
 *
 * @brief LHRS parameters
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef LHRS_PARAMS_HPP
#define LHRS_PARAMS_HPP

#include <iostream>
#include <string>
#include <vector>

#include <reversible/utils/costs.hpp>
#include <reversible/synthesis/lhrs/stg_map_esop.hpp>
#include <reversible/synthesis/lhrs/stg_map_luts.hpp>
#include <reversible/synthesis/lhrs/stg_map_precomp.hpp>
#include <reversible/synthesis/lhrs/stg_map_shannon.hpp>

namespace cirkit
{

enum class lhrs_mapping_strategy
{
  direct,
  lut_based_min_db,
  lut_based_best_fit,
  lut_based_pick_best,
  shannon
};

std::istream& operator>>( std::istream& in, lhrs_mapping_strategy& mapping_strategy );
std::ostream& operator<<( std::ostream& out, const lhrs_mapping_strategy& mapping_strategy );

struct lhrs_params
{
  lhrs_params()
    : map_luts_params( map_esop_params, map_precomp_params ),
      map_shannon_params( map_luts_params )
  {
  }

  unsigned               additional_ancilla = 0u;
  bool                   onlylines          = false;                                       /* do not compute gates */

  lhrs_mapping_strategy  mapping_strategy   = lhrs_mapping_strategy::direct;               /* mapping strategy */
  unsigned               max_func_size      = 0u;                                          /* max function size for DB lookup, 0u: automatic based on class_method */

  stg_map_esop_params         map_esop_params;
  stg_map_precomp_params      map_precomp_params;
  mutable stg_map_luts_params map_luts_params;
  stg_map_shannon_params      map_shannon_params;

  bool                   progress           = false;                                       /* show progress line */
  bool                   verbose            = false;                                       /* be verbose */

  bool                   count_costs        = false;                                       /* DEBUG: count costs and affected lines (for visualization) */

  inline void sync()
  {
    map_esop_params.progress = progress;

    switch ( mapping_strategy )
    {
    default: break;

    case lhrs_mapping_strategy::lut_based_min_db:
      map_luts_params.strategy = stg_map_luts_params::mapping_strategy::mindb;
      break;

    case lhrs_mapping_strategy::lut_based_best_fit:
      map_luts_params.strategy = stg_map_luts_params::mapping_strategy::bestfit;
      break;
    }
  }
};

struct lhrs_stats
{
  lhrs_stats()
    : map_luts_stats( map_esop_stats, map_precomp_stats ),
      map_shannon_stats( map_luts_stats )
  {
  }

  double   runtime           = 0.0;
  double   synthesis_runtime = 0.0;

  unsigned num_decomp_default = 0u;
  unsigned num_decomp_lut     = 0u;

  stg_map_esop_stats    map_esop_stats;
  stg_map_precomp_stats map_precomp_stats;
  stg_map_luts_stats    map_luts_stats;
  stg_map_shannon_stats map_shannon_stats;

  std::vector<cost_t>                gate_costs;
  std::vector<std::vector<unsigned>> line_maps;
  std::vector<std::vector<unsigned>> affected_lines;
  std::vector<std::vector<unsigned>> clean_ancillas;
};

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
