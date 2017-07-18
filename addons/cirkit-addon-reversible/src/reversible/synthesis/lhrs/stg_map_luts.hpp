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
 * @file stg_map_luts.hpp
 *
 * @brief Map single-target gate using minimum datbase LUT mapping
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef STG_MAP_LUTS_HPP
#define STG_MAP_LUTS_HPP

#include <vector>

#include <classical/abc/gia/gia.hpp>
#include <reversible/circuit.hpp>
#include <reversible/synthesis/lhrs/stg_map_esop.hpp>
#include <reversible/synthesis/lhrs/stg_map_precomp.hpp>

namespace cirkit
{

struct stg_map_luts_params
{
  enum class mapping_strategy { mindb, bestfit };

  stg_map_luts_params()
    : own_sub_params( true ),
      map_esop_params( new stg_map_esop_params() ),
      map_precomp_params( new stg_map_precomp_params() )
  {
  }

  stg_map_luts_params( const stg_map_esop_params& map_esop_params,
                       const stg_map_precomp_params& map_precomp_params )
    : own_sub_params( false ),
      map_esop_params( &map_esop_params ),
      map_precomp_params( &map_precomp_params )
  {
  }

  ~stg_map_luts_params()
  {
    if ( own_sub_params )
    {
      delete map_esop_params;
      delete map_precomp_params;
    }
  }

  int              max_cut_size = 4;
  mapping_strategy strategy     = mapping_strategy::bestfit;
  bool             satlut       = false;                     /* perform SAT-based LUT mapping as post-processing step */
  unsigned         area_iters   = 2u;                        /* number of exact area recovery iterations */
  unsigned         flow_iters   = 1u;                        /* number of area flow recovery iterations */

  bool own_sub_params = false;

  stg_map_esop_params const* map_esop_params = nullptr;
  stg_map_precomp_params const* map_precomp_params = nullptr;
};

struct stg_map_luts_stats
{
    stg_map_luts_stats()
    : own_sub_stats( true ),
      map_esop_stats( new stg_map_esop_stats() ),
      map_precomp_stats( new stg_map_precomp_stats() )
  {
  }

  stg_map_luts_stats( stg_map_esop_stats& map_esop_stats,
                       stg_map_precomp_stats& map_precomp_stats )
    : own_sub_stats( false ),
      map_esop_stats( &map_esop_stats ),
      map_precomp_stats( &map_precomp_stats )
  {
  }

  ~stg_map_luts_stats()
  {
    if ( own_sub_stats )
    {
      delete map_esop_stats;
      delete map_precomp_stats;
    }
  }

  double mapping_runtime = 0.0;

  bool own_sub_stats = false;

  stg_map_esop_stats * map_esop_stats = nullptr;
  stg_map_precomp_stats * map_precomp_stats = nullptr;
};

void stg_map_luts( circuit& circ, const gia_graph& function,
                   const std::vector<unsigned>& line_map,
                   const std::vector<unsigned>& ancillas,
                   const stg_map_luts_params& params,
                   stg_map_luts_stats& stats );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
