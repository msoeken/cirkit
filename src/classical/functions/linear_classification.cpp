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

#include "linear_classification.hpp"

#include <cassert>

#include <algorithm>

#include <classical/functions/linear_classification_constants.hpp>
#include <classical/utils/small_truth_table_utils.hpp>
#include <classical/utils/truth_table_utils.hpp>

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

uint64_t exact_linear_classification( uint64_t func, unsigned num_vars )
{
  assert( num_vars >= 2u && num_vars <= 4u );

  const auto offset = 2 * num_vars - 1;

  auto best = func;

  const auto s = linear_classification_constants::masks_start[num_vars - 2u];
  const auto e = linear_classification_constants::masks_end[num_vars - 2u];
  for ( auto i = s; i < e; i += offset )
  {
    uint64_t next{};

    switch ( num_vars )
    {
    case 2u:
      next = stt_permute<2u>( func, &linear_classification_constants::linear_masks[i] );
      break;
    case 3u:
      next = stt_permute<3u>( func, &linear_classification_constants::linear_masks[i] );
      break;
    case 4u:
      next = stt_permute<4u>( func, &linear_classification_constants::linear_masks[i] );
      break;
    }

    best = std::min( next, best );
  }

  return best;
}

uint64_t exact_linear_classification_output( uint64_t func, unsigned num_vars )
{
  const auto mask = num_vars == 6 ? ~0 : ( ( 1 << ( 1 << num_vars ) ) - 1 );
  const auto func_c = ~func & mask;

  return std::min( exact_linear_classification( func, num_vars ), exact_linear_classification( func_c, num_vars ) );
}

uint64_t exact_affine_classification( uint64_t func, unsigned num_vars )
{
  const auto& flip_array = tt_store::i().flips( num_vars );
  const auto total_flips = flip_array.size();

  auto fcopy = func;
  auto best = exact_linear_classification( fcopy, num_vars );

  for ( int j = total_flips - 1; j >= 0; --j )
  {
    fcopy = stt_flip( fcopy, flip_array[j] );
    best = std::min( best, exact_linear_classification( fcopy, num_vars ) );
  }

  return best;
}

uint64_t exact_affine_classification_output( uint64_t func, unsigned num_vars )
{
  const auto mask = num_vars == 6 ? ~0 : ( ( 1 << ( 1 << num_vars ) ) - 1 );
  const auto func_c = ~func & mask;

  return std::min( exact_affine_classification( func, num_vars ), exact_affine_classification( func_c, num_vars ) );
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
