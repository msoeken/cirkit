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
 * @file small_truth_table_utils.hpp
 *
 * @brief Truth table package based on 64-bit integers (up to 6 variables)
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef SMALL_TRUTH_TABLE_UTILS_HPP
#define SMALL_TRUTH_TABLE_UTILS_HPP

#include <cstdint>
#include <cstdlib>
#include <vector>

namespace cirkit
{

namespace stt_constants
{

const uint64_t truths[6] = {0xAAAAAAAAAAAAAAAA, 0xCCCCCCCCCCCCCCCC, 0xF0F0F0F0F0F0F0F0, 0xFF00FF00FF00FF00, 0xFFFF0000FFFF0000, 0xFFFFFFFF00000000};

}

uint64_t stt_flip( uint64_t func, unsigned var );

// see TAOCP 7.1.3-(69)
uint64_t stt_delta_swap( uint64_t func, uint64_t delta, uint64_t omega );

// see TAOCP 7.1.3-(71)
template<std::size_t N>
uint64_t stt_permute( uint64_t func, uint64_t const mask[2 * N - 1] )
{
  for ( auto k = 0u; k < N; ++k )
  {
    func = stt_delta_swap( func, 1 << k, mask[k] );
  }

  for ( int k = N - 2, i = N; k >= 0; --k, ++i )
  {
    func = stt_delta_swap( func, 1 << k, mask[i] );
  }

  return func;
}

std::vector<uint64_t> stt_compute_mask_sequence( const std::vector<unsigned>& perm, unsigned n );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
