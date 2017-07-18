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
 * @file stg_map_esop.hpp
 *
 * @brief Map single-target gate using ESOP minimization
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef STG_MAP_ESOP_HPP
#define STG_MAP_ESOP_HPP

#include <vector>

#include <classical/abc/gia/gia.hpp>
#include <classical/abc/gia/gia_esop.hpp>
#include <classical/optimization/exorcism_minimization.hpp>
#include <reversible/circuit.hpp>

namespace cirkit
{

struct stg_map_esop_params
{
  gia_graph::esop_cover_method cover_method      = gia_graph::esop_cover_method::aig_threshold; /* method to extract initial ESOP cover */
  bool                         optimize_postesop = false;                                       /* post-optimize ESOP cover */
  exorcism_script              script            = exorcism_script::def_wo4;                    /* optimize ESOP synthesized circuit */

  bool                         progress          = false;                                       /* show progress line */

  bool                         nocollapse        = false;                                       /* DEBUG: do not collapse (useful with dumpfile parameter) */
  std::string                  dumpfile;                                                        /* DEBUG: dump ESOP and AIG files for each ESOP cover and LUT */
};

struct stg_map_esop_stats
{
  double   cover_runtime    = 0.0;
  double   exorcism_runtime = 0.0;
  unsigned dumpfile_counter = 0u;
};

void stg_map_esop( circuit& circ, const gia_graph& function,
                   const std::vector<unsigned>& line_map,
                   const stg_map_esop_params& params,
                   stg_map_esop_stats& stats );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
