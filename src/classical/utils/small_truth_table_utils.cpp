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

#include "small_truth_table_utils.hpp"

#include <iostream>

#include <boost/format.hpp>
#include <boost/range/algorithm_ext/iota.hpp>

#include <core/utils/range_utils.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

uint64_t stt_flip( uint64_t func, unsigned var )
{
  return ( ( func << ( 1 << var ) ) & stt_constants::truths[var] ) | ( ( func & stt_constants::truths[var] ) >> ( 1 << var ) );
}

std::pair<uint64_t, uint64_t> stt_compute_mask_pair( std::vector<unsigned>& left,
                                                     std::vector<unsigned>& right,
                                                     unsigned step,
                                                     unsigned n )
{
  const auto loops  = ( 1 << ( n - 1 - step ) );
  const auto diff   = ( 1 << step );

  const auto offset = 1 << ( n - 1 );

  struct node_t
  {
    std::vector<unsigned>::iterator a, b; /* numbers in left or right */
    unsigned lf, rf;                      /* left field right field */
    bool visited = false;
  };

  std::vector<node_t> nodes( 1 << n );

  /* compute graph */
  auto idx1 = 0u, idx2 = 0u;
  auto it1 = std::begin( left );
  for ( auto l1 = 0; l1 < loops; ++l1 )
  {
    for ( auto c1 = 0; c1 < diff; ++c1 )
    {
      auto it2 = std::begin( right );
      idx2 = offset;

      nodes[idx1].a = it1;
      nodes[idx1].b = it1 + diff;

      for ( auto l2 = 0; l2 < loops; ++l2 )
      {
        for ( auto c2 = 0; c2 < diff; ++c2 )
        {
          if ( idx1 == 0u )
          {
            nodes[idx2].a = it2;
            nodes[idx2].b = it2 + diff;
          }

          /* pair elements */
          auto& n1 = nodes[idx1];
          auto& n2 = nodes[idx2];

          /* connect graph */
          if      ( *n1.a == *n2.a ) { n1.lf = idx2; n2.lf = idx1; }
          else if ( *n1.a == *n2.b ) { n1.lf = idx2; n2.rf = idx1; }
          if      ( *n1.b == *n2.a ) { n1.rf = idx2; n2.lf = idx1; }
          else if ( *n1.b == *n2.b ) { n1.rf = idx2; n2.rf = idx1; }

          ++it2; ++idx2;
        }
        it2 += diff;
      }
      ++it1; ++idx1;
    }
    it1 += diff;
  }

  /* traverse graph and compute masks */
  auto mask_left = 0u, mask_right = 0u;

  // std::cout << "[i] left =  " << any_join( left, " " ) << std::endl
  //           << "[i] right = " << any_join( right, " " ) << std::endl;

  while ( true )
  {
    auto idx = 0;

    while ( idx < offset && nodes[idx].visited ) { ++idx; }

    if ( idx == offset ) break;

    auto left_side = true;
    auto nr = *nodes[idx].a;
    auto start = idx;

    do {
      auto& n = nodes[idx];

      // std::cout << boost::format( "[i] enter loop with idx = %d, nr = %d, left_side = %d, *n.a = %d, *n.b = %d, n.lf = %d, n.rf = %d" ) % idx % nr % left_side % *n.a % *n.b % n.lf % n.rf << std::endl;

      auto match = *n.a == nr;

      nr = match ? *n.b : *n.a;
      idx = match ? n.rf : n.lf;
      n.visited = true;

      if ( left_side != match )
      {
        std::swap( *n.a, *n.b );

        if ( left_side )
        {
          mask_left |= 1 << std::distance( left.begin(), n.a );
        }
        else
        {
          mask_right |= 1 << std::distance( right.begin(), n.a );
        }
      }

      left_side = !left_side;

      // std::cout << boost::format( "[i] exit loop with idx = %d, nr = %d, left_side = %d" ) % idx % nr % left_side << std::endl;
    } while ( idx != start );
  }

  return std::make_pair( mask_left, mask_right );
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

uint64_t stt_delta_swap( uint64_t func, uint64_t delta, uint64_t omega )
{
  const uint64_t y = ( func ^ ( func >> delta ) ) & omega;
  return func ^ y ^ ( y << delta );
}

std::vector<uint64_t> stt_compute_mask_sequence( const std::vector<unsigned>& perm, unsigned n )
{
  std::vector<uint64_t> masks( 2 * n - 1, 0u );

  /* initialize left and right */
  std::vector<unsigned> left( perm.size() ), right = perm;
  boost::iota( left, 0u );

  /* all masks except the middle one */
  for ( auto i = 0u; i < n - 1; ++i )
  {
    std::tie( masks[i], masks[2 * n - 2 - i] ) = stt_compute_mask_pair( left, right, i, n );
  }

  // std::cout << "[i] left =  " << any_join( left, " " ) << std::endl
  //           << "[i] right = " << any_join( right, " " ) << std::endl;

  /* compute middle mask */
  for ( auto i = 0u; i < ( 1u << ( n - 1 ) ); ++i )
  {
    if ( left[i] != right[i] )
    {
      masks[n - 1] |= 1 << i;
    }
  }

  return masks;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
