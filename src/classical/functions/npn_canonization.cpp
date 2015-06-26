/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "npn_canonization.hpp"

#include <iostream>
#include <strings.h>

#include <boost/graph/adjacency_list.hpp>
#include <boost/math/special_functions/factorials.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext/iota.hpp>

#include <core/utils/range_utils.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

enum dsd_node_type { dsd_prime, dsd_invert };
typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS,
                              boost::property<boost::vertex_name_t, dsd_node_type>> dsd_graph;
typedef boost::graph_traits<dsd_graph>::vertex_descriptor                           dsd_node;

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

inline dsd_node add_vertex( const dsd_node_type& value, dsd_graph& g )
{
  dsd_node n = boost::add_vertex( g );
  boost::get( boost::vertex_name, g )[n] = value;
  return n;
}

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

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

tt exact_npn_canonization( const tt& t, boost::dynamic_bitset<>& phase, std::vector<unsigned>& perm )
{
  /* initialize */
  auto n = tt_num_vars( t );
  phase.resize( n + 1u );
  perm.resize( n + 1u );
  boost::iota( perm, 0u );

  assert( n <= 6u );

  const auto& swap_array = tt_store::i().swaps( n );
  const auto& flip_array = tt_store::i().flips( n );
  const auto total_swaps = swap_array.size();
  const auto total_flips = flip_array.size();

  auto t1 = t;
  auto t2 = ~t; /* inversion */
  auto min = std::min( t1, t2 );

  if ( false /* verbose */ )
  {
    std::cout << "[i] swaps: " << any_join( swap_array, " " ) << std::endl
              << "[i] flips: " << any_join( flip_array, " " ) << std::endl;
  }

  for ( int i = total_swaps - 1; i >= 0; --i )
  {
    const auto pos = swap_array[i];
    t1 = tt_permute( t1, pos, pos + 1 );
    t2 = tt_permute( t2, pos, pos + 1 );
    tt_shrink( t1, n ); tt_shrink( t2, n );
    std::swap( perm[pos], perm[pos + 1] );
    min = std::min( std::min( t1, t2 ), min );
  }

  for ( int j = total_flips - 1; j >= 0; --j )
  {
    t1 = tt_flip( tt_permute( t1, 0u, 1u ), flip_array[j] );
    t2 = tt_flip( tt_permute( t2, 0u, 1u ), flip_array[j] );
    tt_shrink( t1, n ); tt_shrink( t2, n );
    std::swap( perm[0], perm[1] );
    phase.flip( flip_array[j] );
    min = std::min( std::min( t1, t2 ), min );

    for ( int i = total_swaps - 1; i >= 0; --i )
    {
      const auto pos = swap_array[i];
      t1 = tt_permute( t1, pos, pos + 1 );
      t2 = tt_permute( t2, pos, pos + 1 );
      tt_shrink( t1, n ); tt_shrink( t2, n );
      std::swap( perm[pos], perm[pos + 1] );
      min = std::min( std::min( t1, t2 ), min );
    }
  }

  /* output inverted? */
  phase.set( n, min == t2 );

  return min;
}

tt npn_canonization( const tt& t, boost::dynamic_bitset<>& phase, std::vector<unsigned>& perm )
{
  typedef tt::size_type size_type;

  /* initialize */
  auto n = tt_num_vars( t );
  phase.resize( n + 1u );
  perm.resize( n + 1u );
  boost::iota( perm, 0u );

  tt npn = t;

  auto old_count = npn.size() + 1u;

  while ( old_count != npn.count() )
  {
    old_count = npn.count();

    /* output negation */
    if ( npn.count() > ( npn.size() >> (size_type)1 ) )
    {
      phase.set( n );
      npn.flip();
    }

    /* input negation */
    for ( unsigned i = 0u; i < n; ++i )
    {
      if ( tt_cof1( npn, i ).count() > tt_cof0( npn, i ).count() )
      {
        phase.set( i );
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

  return t;
}

tt npn_canonization_with_dsd( const tt& t, boost::dynamic_bitset<>& phase )
{
  typedef tt::size_type size_type;

  /* phase for all inputs and output */
  unsigned n = tt_num_vars( t );
  phase.resize( n + 1u );

  /* copy truth table to result */
  tt npn = t;

  /* dsd graph */
  dsd_graph g;
  dsd_node node = add_vertex( dsd_prime, g );
  dsd_node root = node;

  /* if there are more 1's than 0's */
  if ( npn.count() > ( npn.size() >> (size_type)1 ) )
  {
    phase.set( n );
    npn.flip();

    root = add_vertex( dsd_invert, g );
    boost::add_edge( root, node, g );
  }

  /* count 1's in cofactors */
  std::vector<int> cof_ones( n );
  for ( unsigned i = 0u; i < n; ++i )
  {
    cof_ones[i] = tt_cof1( npn, i ).count();
  }

  bool try_again = true;

  while ( try_again )
  {
    try_again = false;

    /* decompositio from outside */
    for ( unsigned i = 0u; i < n; ++i )
    {
      if ( !tt_has_var( npn, i ) ) continue;

      char take_cof = 2;

      if ( tt_cof0_is_const1( npn, i ) ) /* dsd: !(x!f1) */
      {
        take_cof = 1;
      }
      else if ( tt_cof0_is_const0( npn, i ) ) /* dsd: (xf1) */
      {
        take_cof = 1;
      }
      else if ( tt_cof1_is_const1( npn, i ) ) /* dsd: !(!x!f1) */
      {
        take_cof = 0;
      }
      else if ( tt_cof1_is_const0( npn, i ) ) /* dsd: (!xf1) */
      {
        take_cof = 0;
      }
      else if ( tt_cofs_opposite( npn, i ) ) /* dsd: [xf0] */
      {
        take_cof = 0;
      }

      if ( take_cof < 2 )
      {
        npn = take_cof ? tt_cof1( npn, i ) : tt_cof0( npn, i );

        npn = tt_remove_var( npn, i );
        --n;
        try_again = true;
      }
    }

    for ( unsigned d = 1; d < n; ++d )
    {
      for ( unsigned i = 0; i < n - d; ++i )
      {
        unsigned j = i + d;

        if ( !tt_has_var( npn, i ) || !tt_has_var( npn, j ) ) continue;

        char take_cof = 2;

        for ( unsigned p = 0; p < 3 && take_cof == 2; ++p )
        {
          if ( p & 0x1 ) { tt_flip( npn, i ); }
          if ( p & 0x2 ) { tt_flip( npn, j ); }

          tt cof00 = tt_cof0( tt_cof0( npn, i ), j );
          tt cof01 = tt_cof1( tt_cof0( npn, i ), j );
          tt cof10 = tt_cof0( tt_cof1( npn, i ), j );
          tt cof11 = tt_cof1( tt_cof1( npn, i ), j );

          if ( cof01 == cof10 )
          {
            if ( cof00 == cof11 )
            {
              take_cof = 0;
            }
            else if ( cof01 == cof11 )
            {
              take_cof = 0;
            }
            else if ( cof00 == cof01 )
            {
              take_cof = 1;
            }
            else
            {
              take_cof = 2;
            }

            if ( take_cof < 2 )
            {
              tt vi = tt_nth_var( i );
              tt_extend( vi, n );

              npn = ~vi & ( take_cof ? cof11 : cof00 ) | ( vi & cof01 );
              npn = tt_remove_var( npn, j );
              --n;

              try_again = true;
            }
          }

          if ( take_cof == 2 )
          {
            if ( p & 0x1 ) { tt_flip( npn, i ); }
            if ( p & 0x2 ) { tt_flip( npn, j ); }

            if ( tt_cof1( npn, i ).count() > tt_cof1( npn, j ).count() )
            {
              npn = tt_permute( npn, i, j );
            }
          }
        }
      }
    }
  }

  return npn;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
