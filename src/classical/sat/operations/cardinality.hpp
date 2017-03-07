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
 * @file cardinality.hpp
 *
 * @brief SAT cardinality constraints
 *
 * @author Mathias Soeken
 * @author Heinz Riener
 *
 * @since  2.2
 */

#ifndef CARDINALITY_HPP
#define CARDINALITY_HPP

#include <numeric>

#include <boost/assign/std/vector.hpp>
#include <boost/range.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm_ext/iota.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>

#include <core/utils/range_utils.hpp>
#include <classical/sat/sat_solver.hpp>
#include <classical/sat/operations/logic.hpp>

using namespace boost::assign;

/**
 * Carsten Sinz's CNF encoding of Boolean cardinality constraints [1].
 * The implementation is mainly based on the Knuth's descriptions [2].
 *
 * [1] Carsten Sinz. Towards an optimal CNF encoding of boolean
 * cardinality constraints, CP, pages 827-831, 2005.
 *
 * [2] Donald E. Knuth, The Art of Computer Programming, Volume 4,
 * Pre-Fascicle 6A, page 8ff.
 */

namespace cirkit
{

template<class S, class C>
void atmost_one( S& solver, const C& x )
{
  for ( auto i = boost::begin( x ); i != boost::end( x ); ++i )
  {
    if ( i + 1 == boost::end( x ) ) continue;
    for ( auto j = i + 1; j != boost::end( x ); ++j )
    {
      add_clause( solver )( {-*i, -*j} );
    }
  }
}

template<class S, class C>
inline void one_hot( S& solver, const C& x )
{
  add_clause( solver )( x );
  atmost_one( solver, x );
}

template < class S >
int less_or_equal_sinz( S& solver, const clause_t& x, unsigned r, int sid, int spol = 1 )
{
  const unsigned n = x.size();

  //std::cerr << "n = " << n << " r = " << r << " sid = " << sid << '\n';
  assert( n > r );
  for ( auto j = 0u; j < n - r - 1; ++j )
  {
    for ( auto k = 0u; k < r; ++k )
    {
      // std::cerr << "(" << j << "," << k << ")" << '\n';
      //const int l1 = spol * -(sid + j * (n - r) + k);
      //const int l2 = spol * (sid + (j + 1u) * (n - r) + k);
      const int l1 = spol * -( sid + j * r + k );
      const int l2 = spol * (sid + ( j + 1u ) * r + k );
      add_clause( solver )( {l1, l2} );
    }
  }

  for ( auto j = 0u; j < (n - r); ++j )
  {
    for ( auto k = 0u; k <= r; ++k )
    {
      // std::cerr << "(" << j << "," << k << ")" << '\n';
      clause_t clause;
      clause += -x[ j + k ];
      //if ( k > 0u ) clause += spol * -(sid + j * ( n - r ) + k - 1u );
      //if ( k < r ) clause += spol * (sid + j * ( n - r ) + k );
      if ( k > 0u ) clause += spol * -(sid + j * r + k - 1u );
      if ( k < r ) clause += spol * (sid + j * r + k );
      add_clause( solver )( clause );
    }
  }

  sid += (n - r)*r/* + 1u*/;
  //std::cerr << "next sid: " << sid << '\n';
  return sid;
}

template < class S >
int greater_or_equal_sinz( S& solver, const clause_t& x, unsigned r, int sid, int spol = 1 )
{
  using boost::adaptors::transformed;

  clause_t xn;
  boost::push_back( xn, x | transformed( []( int v ) { return -v; } ) );
  return less_or_equal_sinz( solver, xn, x.size() - r, sid, spol );
}

template < class S >
int equals_sinz( S& solver, const clause_t& x, unsigned r, int sid )
{
  //std::cout << "call equals_size with x.size = " << x.size() << ", r = " << r << ", and sid = " << sid << std::endl;

  if ( r == 0u )
  {
    for ( const auto& l : x )
    {
      add_clause( solver )( {-l} );
    }
    return sid;
  }

  if ( r == x.size() )
  {
    for ( const auto& l : x )
    {
      add_clause( solver )( {l} );
    }
    return sid;
  }

  sid = less_or_equal_sinz( solver, x, r, sid, 1 );
  return greater_or_equal_sinz( solver, x, r, sid, -1 );
}

/******************************************************************************
 * Binary tree encoding from Bailleux and Boufkhad                            *
 ******************************************************************************/
template<class S>
int less_or_equal_bailleux_boufkhad_assume( S& solver, int assume, const clause_t& x, unsigned r, int sid )
{
  const auto n = x.size();                    /* number of variables */
  std::vector<unsigned> ts_count( 2 * n, 1 ); /* pre-assign t's with 1 (for the leaves), we don't use index 0 */
  std::vector<int> ts( 2 * n, 0 );
  std::vector<int> pol( 2 * n, 1 );

  auto offset = sid;
  for ( auto k = n - 1; k > 1; --k )
  {
    ts_count[k] = std::min( ts_count[k << 1] + ts_count[( k << 1 ) + 1], r );
    ts[k] = offset;
    offset += ts_count[k];
  }
  for ( auto k = n; k < 2 * n; ++k )
  {
    ts[k] = abs( x[k - n] );
    pol[k] = x[k - n] > 0 ? 1 : -1;
  }

  for ( auto k = 2u; k < n; ++k )
  {
    for ( auto i = 0u; i <= ts_count[k << 1]; ++i )
    {
      for ( auto j = 0u; j <= ts_count[( k << 1 ) + 1]; ++j )
      {
        if ( ( i + j < 1 ) || ( i + j > ts_count[k] + 1 ) ) { continue; }

        std::vector<int> clause = { -assume };
        if ( i > 0 ) { clause.push_back( -pol[k << 1] * ( ts[k << 1] + i - 1 ) ); }
        if ( j > 0 ) { clause.push_back( -pol[( k << 1 ) + 1] * ( ts[( k << 1 ) + 1] + j - 1 ) ); }
        if ( i + j <= r ) { clause.push_back( pol[k] * ( ts[k] + i + j - 1 ) ); }

        if ( !clause.empty() )
        {
          add_clause( solver )( clause );
        }
      }
    }
  }

  for ( auto i = 0; i <= static_cast<int>( ts_count[2] ); ++i )
  {
    for ( auto j = 0; j <= static_cast<int>( ts_count[3] ); ++j )
    {
      if ( i + j == static_cast<int>( r ) + 1 )
      {
        assert( i && j );
        add_clause( solver )( { -assume, -pol[2] * ( ts[2] + i - 1 ), -pol[3] * ( ts[3] + j - 1 )} );
      }
    }
  }

  return offset;
}

template<class S>
int less_or_equal_bailleux_boufkhad( S& solver, const clause_t& x, unsigned r, int sid )
{
  const auto n = x.size();                    /* number of variables */
  std::vector<unsigned> ts_count( 2 * n, 1 ); /* pre-assign t's with 1 (for the leaves), we don't use index 0 */
  std::vector<int> ts( 2 * n, 0 );
  std::vector<int> pol( 2 * n, 1 );

  auto offset = sid;
  for ( int k = n - 1; k > 1; --k )
  {
    ts_count[k] = std::min( ts_count[k << 1] + ts_count[( k << 1 ) + 1], r );
    ts[k] = offset;
    offset += ts_count[k];
  }
  for ( auto k = n; k < 2 * n; ++k )
  {
    ts[k] = abs( x[k - n] );
    pol[k] = x[k - n] > 0 ? 1 : -1;
  }

  for ( auto k = 2u; k < n; ++k )
  {
    for ( auto i = 0u; i <= ts_count[k << 1]; ++i )
    {
      for ( auto j = 0u; j <= ts_count[( k << 1 ) + 1]; ++j )
      {
        if ( ( i + j < 1 ) || ( i + j > ts_count[k] + 1 ) ) { continue; }

        std::vector<int> clause;
        if ( i > 0 ) { clause.push_back( -pol[k << 1] * ( ts[k << 1] + i - 1 ) ); }
        if ( j > 0 ) { clause.push_back( -pol[( k << 1 ) + 1] * ( ts[( k << 1 ) + 1] + j - 1 ) ); }
        if ( i + j <= r ) { clause.push_back( pol[k] * ( ts[k] + i + j - 1 ) ); }

        if ( !clause.empty() )
        {
          add_clause( solver )( clause );
        }
      }
    }
  }

  for ( auto i = 0; i <= static_cast<int>( ts_count[2] ); ++i )
  {
    for ( auto j = 0; j <= static_cast<int>( ts_count[3] ); ++j )
    {
      if ( i + j == static_cast<int>( r ) + 1 )
      {
        assert( i && j );
        add_clause( solver )( {-pol[2] * ( ts[2] + i - 1 ), -pol[3] * ( ts[3] + j - 1 )} );
      }
    }
  }

  return offset;
}

template<class S>
int greater_or_equal_bailleux_boufkhad( S& solver, const clause_t& x, unsigned r, int sid )
{
  using boost::adaptors::transformed;

  clause_t xn;
  boost::push_back( xn, x | transformed( []( int v ) { return -v; } ) );
  return less_or_equal_bailleux_boufkhad( solver, xn, x.size() - r, sid );
}

template < class S >
int equals_bailleux_boufkhad( S& solver, const clause_t& x, unsigned r, int sid )
{
  if ( r == 0u )
  {
    for ( const auto& l : x )
    {
      add_clause( solver )( {-l} );
    }
    return sid;
  }

  if ( r == x.size() )
  {
    for ( const auto& l : x )
    {
      add_clause( solver )( {l} );
    }
    return sid;
  }

  sid = less_or_equal_bailleux_boufkhad( solver, x, r, sid );
  return greater_or_equal_bailleux_boufkhad( solver, x, r, sid );
}

/******************************************************************************
 * Pairwise cardinality networks                                              *
 ******************************************************************************/

// Based on [M. Codish and M. Zazon-Ivry, LPAR, 2016, 154-172] and https://bitbucket.org/alanmi/abc/src/f78c2854aa5948fea93533c3934fb0f6a2c3c785/src/aig/gia/giaSatMap.c?at=default&fileviewer=file-view-default

// namespace detail
// {

// template<class S>
// void sat_add_half_sorter( S& solver, int a, int b, int y, int z )
// {
//   add_clause( solver )( {a, -y} );
//   add_clause( solver )( {a, -z} );
//   add_clause( solver )( {b, -y, -z} );
//   // add_clause( solver )( {-a, y} );
//   // add_clause( solver )( {-a, z} );
//   // add_clause( solver )( {-b, y, z} );
// }

// template<class S>
// void sat_pairwise_cardinality_network_add_sorter( S& solver, std::vector<int>& vvars, int i, int k, int *pnvars )
// {
//   const auto ivar1 = (*pnvars)++;
//   const auto ivar2 = (*pnvars)++;

//   sat_add_half_sorter( solver, ivar1, ivar2, vvars[i], vvars[k] );
//   vvars[i] = ivar1;
//   vvars[k] = ivar2;
// }

// template<class S>
// void sat_pairwise_cardinality_network_add_constraint_merge( S& solver, std::vector<int>& vvars, int lo, int hi, int r, int *pnvars )
// {
//   const auto step = r * 2;
//   if ( step >= hi - lo ) return;

//   sat_pairwise_cardinality_network_add_constraint_merge( solver, vvars, lo, hi - r, step, pnvars );
//   sat_pairwise_cardinality_network_add_constraint_merge( solver, vvars, lo + r, hi, step, pnvars );
//   for ( auto i = lo + r; i < hi - r; i += step )
//   {
//     sat_pairwise_cardinality_network_add_sorter( solver, vvars, i, i + r, pnvars );
//   }
// }

// template<class S>
// void sat_pairwise_cardinality_network_add_constraint_range( S& solver, std::vector<int>& vvars, int lo, int hi, int *pnvars )
// {
//   if ( hi - lo < 1 ) return;

//   const auto mid = lo + ( hi - lo ) / 2;
//   for ( auto i = lo; i <= mid; ++i )
//   {
//     sat_pairwise_cardinality_network_add_sorter( solver, vvars, i, i + ( hi - lo + 1 ) / 2, pnvars );
//   }
//   sat_pairwise_cardinality_network_add_constraint_range( solver, vvars, lo, mid, pnvars );
//   sat_pairwise_cardinality_network_add_constraint_range( solver, vvars, mid + 1, hi, pnvars );
//   sat_pairwise_cardinality_network_add_constraint_merge( solver, vvars, lo, hi, 1, pnvars );
// }

// template<class S>
// int sat_pairwise_cardinality_network_add_constraint_pairwise( S& solver, std::vector<int>& vvars )
// {
//   int nvars = vvars.size();
//   sat_pairwise_cardinality_network_add_constraint_range( solver, vvars, 0, nvars - 1, &nvars );
//   return nvars;
// }

// }

// template<class S>
// int sat_pairwise_cardinality_network2( S& solver, unsigned log_n, int sid, std::vector<int>* ovars = nullptr )
// {
//   auto nvars = 1u << log_n;
//   const auto nvars_alloc = nvars + 2u * (nvars * log_n * ( log_n - 1 ) / 4 + nvars - 1);

//   std::vector<int> vvars( nvars );
//   boost::iota( vvars, sid );

//   std::cout << "[i] original vvars: " << any_join( vvars, " " ) << std::endl;

//   const auto nvars_real = detail::sat_pairwise_cardinality_network_add_constraint_pairwise( solver, vvars );
//   assert( nvars_real == nvars_alloc );

//   if ( ovars )
//   {
//     *ovars = vvars;
//   }

//   return sid + nvars_alloc;
// }

/******************************************************************************
 * PW implementation based on paper                                           *
 ******************************************************************************/

namespace detail
{

template<class S>
void sat_pairwise_comparator( S& solver, int a, int b, int c, int d)
{
  //logic_or( solver, a, b, c );
  //logic_and( solver, a, b, d );

  add_clause( solver )( {c, -a} );
  add_clause( solver )( {c, -b} );
  add_clause( solver )( {d, -a, -b} );
}

template<class S>
void sat_pairwise_split( S& solver, const std::vector<int>& as, const std::vector<int>& bs, const std::vector<int>& cs )
{
  assert( bs.size() == cs.size() );
  assert( bs.size() << 1u == as.size() );

  for ( auto i = 0u; i < bs.size(); ++i )
  {
    sat_pairwise_comparator( solver, as[2 * i], as[2 * i + 1], bs[i], cs[i] );
  }
}

template<class S>
int sat_pairwise_merge( S& solver, const std::vector<int>& as, const std::vector<int>& bs, const std::vector<int>& cs, int sid )
{
  const auto n = as.size();

  if ( n == 1u )
  {
    equals( solver, as[0], cs[0] );
    equals( solver, bs[0], cs[1] );
    return sid;
  }

  /* fill ds and es */
  std::vector<int> ds( n ), es( n );

  ds[0u] = cs[0];
  for ( auto i = 1u; i < n; ++i ) { ds[i] = sid++; }

  for ( auto i = 0u; i < n - 1; ++i ) { es[i] = sid++; }
  es[n - 1] = cs[2 * n - 1];

  /* fill a0, b0, a1, b1 */
  std::vector<int> a0( n / 2 ), b0( n / 2 ), a1( n / 2 ), b1( n / 2 );
  for ( auto i = 0u; i < n / 2; ++i )
  {
    a0[i] = as[2 * i];
    a1[i] = as[2 * i + 1];
    b0[i] = bs[2 * i];
    b1[i] = bs[2 * i + 1];
  }

  sid = sat_pairwise_merge( solver, a0, b0, ds, sid );
  sid = sat_pairwise_merge( solver, a1, b1, es, sid );

  for ( auto i = 0u; i < n - 1; ++i )
  {
    sat_pairwise_comparator( solver, es[i], ds[i + 1], cs[2 * i + 1], cs[2 * i + 2] );
  }

  /* sort outputs */
  for ( auto i = 0u; i < cs.size() - 1; ++i )
  {
    add_clause( solver )( {cs[i], -cs[i + 1]} );
  }

  return sid;
}

template<class S>
int sat_pairwise_sort( S& solver, const std::vector<int>& as, const std::vector<int>& ds, int sid )
{
  const auto n = as.size();

  if ( n == 1u )
  {
    equals( solver, as[0], ds[0] );
    return sid;
  }

  std::vector<int> bs( n / 2 ), cs( n / 2 ), bbs( n / 2), ccs( n / 2 );

  boost::iota( bs, sid ); sid += n / 2;
  boost::iota( cs, sid ); sid += n / 2;
  boost::iota( bbs, sid ); sid += n / 2;
  boost::iota( ccs, sid ); sid += n / 2;

  sat_pairwise_split( solver, as, bs, cs );
  sid = sat_pairwise_sort( solver, bs, bbs, sid );
  sid = sat_pairwise_sort( solver, cs, ccs, sid );
  return sat_pairwise_merge( solver, bbs, ccs, ds, sid );
}

}

template<class S>
int sat_pairwise_cardinality_network( S& solver, const std::vector<int>& ivars, int sid, std::vector<int>* ovars = nullptr )
{
  std::vector<int> ds( ivars.size() );
  boost::iota( ds, sid );
  sid += ivars.size();

  if ( ovars )
  {
    *ovars = ds;
  }

  return detail::sat_pairwise_sort( solver, ivars, ds, sid );
}

template<class S>
int sat_pairwise_cardinality_network( S& solver, unsigned log_n, int sid, std::vector<int>* ovars = nullptr )
{
  const auto n = 1 << log_n;
  std::vector<int> as( n );

  boost::iota( as, sid ); sid += n;

  return sat_pairwise_cardinality_network( solver, as, sid, ovars );
}

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
