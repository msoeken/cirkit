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

#include <classical/abc/gia/gia.hpp>
#include <classical/abc/gia/gia_esop.hpp>
#include <classical/optimization/exorcism_minimization.hpp>
#include <reversible/utils/costs.hpp>

namespace cirkit
{

enum class lhrs_mapping_strategy
{
  direct,
  lut_based_min_db,
  lut_based_best_fit,
  lut_based_pick_best
};

std::istream& operator>>( std::istream& in, lhrs_mapping_strategy& mapping_strategy );
std::ostream& operator<<( std::ostream& out, const lhrs_mapping_strategy& mapping_strategy );

struct lhrs_params
{
  unsigned                     additional_ancilla = 0u;
  bool                         onlylines          = false;                                       /* do not compute gates */

  gia_graph::esop_cover_method cover_method       = gia_graph::esop_cover_method::aig_threshold; /* method to extract initial ESOP cover */
  bool                         optimize_postesop  = false;                                       /* post-optimize ESOP cover */
  exorcism_script              script             = exorcism_script::def_wo4;                    /* optimize ESOP synthesized circuit */

  lhrs_mapping_strategy        mapping_strategy   = lhrs_mapping_strategy::direct;               /* mapping strategy */
  bool                         satlut             = false;                                       /* perform SAT-based LUT mapping as post-processing step */
  unsigned                     area_iters         = 2u;                                          /* number of exact area recovery iterations */
  unsigned                     flow_iters         = 1u;                                          /* number of area flow recovery iterations */
  unsigned                     class_method       = 0u;                                          /* classification method: 0u: spectral, 1u: affine */
  unsigned                     max_func_size      = 0u;                                          /* max function size for DB lookup, 0u: automatic based on class_method */

  bool                         progress           = false;                                       /* show progress line */
  bool                         verbose            = false;                                       /* be verbose */

  bool                         nocollapse         = false;                                       /* DEBUG: do not collapse (useful with dumpfile parameter) */
  bool                         count_costs        = false;                                       /* DEBUG: count costs and affected lines (for visualization) */
  std::string                  dumpfile;                                                         /* DEBUG: dump ESOP and AIG files for each ESOP cover and LUT */
};

struct lhrs_stats
{
  lhrs_stats()
    : class_counter( 4u )
  {
    class_counter[0u].resize( 3u );
    class_counter[1u].resize( 6u );
    class_counter[2u].resize( 18u );
    class_counter[3u].resize( 48u );
  }

  double   runtime           = 0.0;
  double   synthesis_runtime = 0.0;
  double   exorcism_runtime  = 0.0;
  double   cover_runtime     = 0.0;
  double   mapping_runtime   = 0.0;
  double   class_runtime     = 0.0;
  unsigned dumpfile_counter  = 0u;

  unsigned num_decomp_default = 0u;
  unsigned num_decomp_lut     = 0u;

  std::vector<std::vector<unsigned>> class_counter;
  std::vector<cost_t>                gate_costs;
  std::vector<std::vector<unsigned>> clean_ancillas;
  std::vector<std::vector<unsigned>> line_maps;
};

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
