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

#include "npn_canonization.hpp"

#include <iostream>
#include <strings.h>

#include <boost/graph/adjacency_list.hpp>
#include <boost/math/special_functions/factorials.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext/iota.hpp>

#include <core/utils/range_utils.hpp>
#include <core/utils/timer.hpp>
#include <classical/abc/abc_api.hpp>

#include <misc/util/utilTruth.h>
#include <bool/lucky/lucky.h>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

std::vector<unsigned> compute_swaps( unsigned n )
{
  const unsigned N = static_cast<unsigned>( boost::math::factorial<double>( n ) );
  std::vector<unsigned> swap_array( N - 1u );
  auto d = N / 2u;
  swap_array[d - 1u] = 0u;
  auto m = 2u;
  unsigned j, k;

  while ( m != n )
  {
    ++m;
    d /= m;
    k = 0;

    do
    {
      k += d;
      j = m - 1u;
      while ( j > 0u )
      {
        swap_array[k - 1u] = --j;
        k += d;
      }
      swap_array[k - 1u]++;
      k += d;
      while ( j < m - 1u )
      {
        swap_array[k - 1u] = j++;
        k += d;
      }
    } while ( k < N );
  }

  return swap_array;
}

std::vector<unsigned> compute_flips( unsigned n )
{
  const auto total_flips = ( 1u << n ) - 1;
  std::vector<unsigned> flip_array( total_flips );

  auto graynumber = 0u;
  auto temp = 0u;
  for ( auto i = 1u; i <= total_flips; ++i )
  {
    graynumber = i ^ (i >> 1);
    flip_array[total_flips - i] = ffs( temp ^ graynumber ) - 1u;
    temp = graynumber;
  }

  return flip_array;
}

inline void bitset_swap( boost::dynamic_bitset<>& bs, unsigned i, unsigned j )
{
  auto t = bs[i];
  bs[i] = bs[j];
  bs[j] = t;
}

void npn_canonization_sifting_loop( tt& npn, unsigned n, boost::dynamic_bitset<>& phase, std::vector<unsigned>& perm )
{
  auto improvement = true;
  auto forward = true;

  //auto round = 0u;

  while ( improvement )
  {
    //std::cout << "[i] round " << ++round << std::endl;

    improvement = false;

    for ( int i = forward ? 0 : n - 2; forward ? i < static_cast<int>( n - 1 ) : i >= 0; forward ? ++i : --i )
    {
      auto local_improvement = false;
      for ( auto k = 1u; k < 8u; ++k )
      {
        if ( k % 4u == 0u )
        {
          const auto next_t = tt_permute( npn, i, i + 1 );
          if ( next_t < npn )
          {
            npn = next_t;
            std::swap( perm[i], perm[i + 1] );
            local_improvement = true;
          }
        }
        else if ( k % 2u == 0u )
        {
          const auto next_t = tt_flip( npn, i + 1 );
          if ( next_t < npn )
          {
            npn = next_t;
            phase.flip( i + 1 );
            local_improvement = true;
          }
        }
        else
        {
          const auto next_t = tt_flip( npn, i );
          if ( next_t < npn )
          {
            npn = next_t;
            phase.flip( i );
            local_improvement = true;
          }
        }
      }

      if ( local_improvement )
      {
        improvement = true;
      }
      else
      {
        std::swap( perm[i], perm[i + 1] );
      }
    }

    forward = !forward;
  }
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

tt exact_npn_canonization( const tt& t, boost::dynamic_bitset<>& phase, std::vector<unsigned>& perm, const properties::ptr& settings, const properties::ptr& statistics )
{
  properties_timer tim( statistics );

  /* initialize */
  auto n = tt_num_vars( t );
  phase.resize( n + 1u );
  phase.reset();
  perm.resize( n );
  boost::iota( perm, 0u );

  assert( n <= 6u );

  const auto& swap_array = tt_store::i().swaps( n );
  const auto& flip_array = tt_store::i().flips( n );
  const auto total_swaps = swap_array.size();
  const auto total_flips = flip_array.size();

  auto t1 = t;
  auto t2 = ~t; /* inversion */
  auto min = std::min( t1, t2 );

  auto invo = ( min == t2 );

  if ( false /* verbose */ )
  {
    std::cout << "[i] swaps: " << any_join( swap_array, " " ) << std::endl
              << "[i] flips: " << any_join( flip_array, " " ) << std::endl;
  }

  int best_flip = total_flips;
  int best_swap = total_swaps;

  for ( int i = total_swaps - 1; i >= 0; --i )
  {
    const auto pos = swap_array[i];
    t1 = tt_permute( t1, pos, pos + 1 );
    t2 = tt_permute( t2, pos, pos + 1 );
    tt_shrink( t1, n ); tt_shrink( t2, n );
    if ( t1 < min || t2 < min )
    {
      best_swap = i;
      min = std::min( std::min( t1, t2 ), min );
      invo = ( min == t2 );
    }
  }

  for ( int j = total_flips - 1; j >= 0; --j )
  {
    t1 = tt_flip( tt_permute( t1, 0u, 1u ), flip_array[j] );
    t2 = tt_flip( tt_permute( t2, 0u, 1u ), flip_array[j] );
    tt_shrink( t1, n ); tt_shrink( t2, n );
    if ( t1 < min || t2 < min )
    {
      best_swap = total_swaps;
      best_flip = j;
      min = std::min( std::min( t1, t2 ), min );
      invo = ( min == t2 );
    }

    for ( int i = total_swaps - 1; i >= 0; --i )
    {
      const auto pos = swap_array[i];
      t1 = tt_permute( t1, pos, pos + 1 );
      t2 = tt_permute( t2, pos, pos + 1 );
      tt_shrink( t1, n ); tt_shrink( t2, n );
      if ( t1 < min || t2 < min )
      {
        best_swap = i;
        best_flip = j;
        min = std::min( std::min( t1, t2 ), min );
        invo = ( min == t2 );
      }
    }
  }

  for ( int i = total_swaps - 1; i >= best_swap; --i )
  {
    const auto pos = swap_array[i];
    std::swap( perm[pos], perm[pos + 1] );
  }

  for ( int j = total_flips - 1; j >= best_flip; --j )
  {
    phase.flip( flip_array[j] );
  }

  /* output inverted? */
  if ( invo )
  {
    phase.flip( n );
  }

  return min;
}

tt npn_canonization( const tt& t, boost::dynamic_bitset<>& phase, std::vector<unsigned>& perm, const properties::ptr& settings, const properties::ptr& statistics )
{
  properties_timer tim( statistics );

  typedef tt::size_type size_type;

  /* initialize */
  auto n = tt_num_vars( t );
  phase.resize( n + 1u );
  phase.reset();
  perm.resize( n );
  boost::iota( perm, 0u );

  tt npn = t;

  auto old_count = npn.size() + 1u;

  while ( old_count != npn.count() )
  {
    old_count = npn.count();

    /* output negation */
    if ( npn.count() > ( npn.size() >> (size_type)1 ) )
    {
      phase.flip( n );
      npn.flip();
    }

    /* input negation */
    for ( unsigned i = 0u; i < n; ++i )
    {
      if ( tt_cof1( npn, i ).count() > tt_cof0( npn, i ).count() )
      {
        phase.flip( i );
        npn = tt_flip( npn, i );
      }
    }

    /* permute inputs */
    for ( unsigned d = 1u; d < n - 1; ++d )
    {
      for ( unsigned i = 0u; i < n - d; ++i )
      {
        unsigned j = i + d;

        if ( tt_cof1( npn, i ).count() > tt_cof1( npn, j ).count() )
        {
          npn = tt_permute( npn, i, j );
          std::swap( perm[i], perm[j] );
        }
      }
    }
  }

  if ( tt_num_vars( npn ) > n )
  {
    tt_shrink( npn, n );
  }

  return npn;
}

tt npn_canonization_flip_swap( const tt& t, boost::dynamic_bitset<>& phase, std::vector<unsigned>& perm, const properties::ptr& settings, const properties::ptr& statistics )
{
  properties_timer tim( statistics );

  /* initialize */
  auto n = tt_num_vars( t );
  phase.resize( n + 1u );
  phase.reset();
  perm.resize( n );
  boost::iota( perm, 0u );

  tt npn = t;

  auto improvement = true;

  //auto round = 0u;

  while ( improvement )
  {
    //std::cout << "[i] round " << ++round << std::endl;

    improvement = false;

    /* input inversion */
    for ( auto i = 0u; i < n; ++i )
    {
      const auto flipped = tt_flip( npn, i );
      if ( flipped < npn )
      {
        //std::cout << "[i]   flip input " << i << std::endl;
        npn = flipped;
        phase.flip( i );
        improvement = true;
      }
    }

    /* output inversion */
    const auto flipped = ~npn;
    if ( flipped < npn )
    {
      //std::cout << "[i]   flip output" << std::endl;
      npn = flipped;
      phase.flip( n );
      improvement = true;
    }

    /* permute inputs */
    for ( auto d = 1u; d < n - 1; ++d )
    {
      for ( auto i = 0u; i < n - d; ++i )
      {
        auto j = i + d;

        const auto permuted = tt_permute( npn, i, j );
        if ( permuted < npn )
        {
          //std::cout << "[i]   swap inputs " << i << " and " << j << std::endl;
          npn = permuted;
          std::swap( perm[i], perm[j] );
          bitset_swap( phase, i, j );
          improvement = true;
        }
      }
    }
  }

  if ( tt_num_vars( npn ) > n )
  {
    tt_shrink( npn, n );
  }

  return npn;
}

tt npn_canonization_sifting( const tt& t, boost::dynamic_bitset<>& phase, std::vector<unsigned>& perm, const properties::ptr& settings, const properties::ptr& statistics )
{
  properties_timer tim( statistics );

  /* initialize */
  auto n = tt_num_vars( t );
  phase.resize( n + 1u );
  phase.reset();
  perm.resize( n );
  boost::iota( perm, 0u );

  tt npn = t;

  if ( n < 2u )
  {
    return npn;
  }

  npn_canonization_sifting_loop( npn, n, phase, perm );

  const auto best_perm = perm;
  const auto best_phase = phase;
  const auto best_npn = npn;

  npn = ~t;
  phase.reset();
  phase.flip( n );
  boost::iota( perm, 0u );

  npn_canonization_sifting_loop( npn, n, phase, perm );

  if ( best_npn < npn )
  {
    perm = best_perm;
    phase = best_phase;
    npn = best_npn;
  }

  if ( tt_num_vars( npn ) > n )
  {
    tt_shrink( npn, n );
  }

  return npn;
}

tt tt_from_npn( const tt& npn, const boost::dynamic_bitset<>& phase, std::vector<unsigned>& perm )
{
  tt t = npn;

  /* permute inputs */
  for ( unsigned i = 0; i < perm.size(); ++i )
  {
    if ( perm[i] == i ) continue;

    unsigned pos = boost::find( perm, i ) - perm.begin();
    t = tt_permute( t, i, pos );
    std::swap( perm[i], perm[pos] );
  }

  /* invert inputs */
  for ( unsigned i = 0; i < perm.size(); ++i )
  {
    if ( phase.test( i ) )
    {
      t = tt_flip( t, i );
    }
  }

  /* invert output */
  if ( phase.test( perm.size() ) )
  {
    t.flip();
  }

  if ( tt_num_vars( t ) > perm.size() )
  {
    tt_shrink( t, perm.size() );
  }

  return t;
}

tt npn_canonization_lucky( const tt& t, boost::dynamic_bitset<>& phase, std::vector<unsigned>& perm )
{
  std::vector<unsigned long> truth( t.num_blocks() );
  boost::to_block_range( t, &truth[0] );

  const auto num_vars = tt_num_vars( t );

  char pCanonPerm[16];
  abc::resetPCanonPermArray( pCanonPerm, num_vars );
  auto uCanonPhase = abc::luckyCanonicizer_final_fast( &truth[0], num_vars, pCanonPerm );

  boost::dynamic_bitset<> phase_direct( num_vars + 1, uCanonPhase );
  phase = phase_direct;

  perm.resize( num_vars );
  for ( auto i = 0u; i < num_vars; ++i )
  {
    perm[i] = pCanonPerm[i] - 'a';

    if ( perm[i] >= num_vars )
    {
      std::cout << "[e] problem with canonization" << std::endl;
      std::cout << "[e] pCanonPerm:";
      for ( auto i = 0u; i < num_vars; ++i )
      {
        std::cout << " " << pCanonPerm[i];
      }
      std::cout << std::endl;
      std::cout << "[e] truth table: " << t << std::endl;
      std::cout << "[e] tt num vars: " << tt_num_vars( t ) << std::endl;
      assert( false );
    }
    phase[perm[i]] = phase_direct[i];
  }

  return tt( truth.begin(), truth.end() );
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
