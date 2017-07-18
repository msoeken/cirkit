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
 * @file stg_map_precomp.hpp
 *
 * @brief Map single-target gate using precomputed results
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef STG_MAP_PRECOMP_HPP
#define STG_MAP_PRECOMP_HPP

#include <cinttypes>
#include <unordered_map>
#include <vector>

#include <classical/utils/truth_table_utils.hpp>
#include <reversible/circuit.hpp>

namespace cirkit
{

struct stg_map_precomp_params
{
  unsigned                     class_method       = 0u;                                          /* classification method: 0u: spectral, 1u: affine */
};

struct stg_map_precomp_stats
{
  stg_map_precomp_stats()
    : class_counter( 4u ),
      class_hash( 4u )
  {
    class_counter[0u].resize( 3u );
    class_counter[1u].resize( 6u );
    class_counter[2u].resize( 18u );
    class_counter[3u].resize( 48u );
  }

  double   class_runtime     = 0.0;

  std::vector<std::vector<unsigned>> class_counter;

  std::vector<std::unordered_map<uint64_t, uint64_t>> class_hash;
};

void stg_map_precomp( circuit& circ, uint64_t function, unsigned num_vars,
                      const std::vector<unsigned>& line_map,
                      const stg_map_precomp_params& params,
                      stg_map_precomp_stats& stats );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
