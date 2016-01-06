/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2016  EPFL
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

#include <boost/assign/std/vector.hpp>
#include <boost/range.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>

#include <formal/sat/sat_solver.hpp>

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
unsigned less_or_equal_sinz( S& solver, const clause_t& x, unsigned r, int sid, int spol = 1 )
{
  const unsigned n = x.size();

  //std::cerr << "n = " << n << " r = " << r << " sid = " << sid << '\n';
  assert( n > r );
  for ( int j = 0; j < n - r - 1; ++j )
  {
    for ( int k = 0; k < r; ++k )
    {
      // std::cerr << "(" << j << "," << k << ")" << '\n';
      //const int l1 = spol * -(sid + j * (n - r) + k);
      //const int l2 = spol * (sid + (j + 1u) * (n - r) + k);
      const int l1 = spol * -( sid + j * r + k );
      const int l2 = spol * (sid + ( j + 1u ) * r + k );
      add_clause( solver )( {l1, l2} );
    }
  }

  for ( int j = 0; j < (n - r); ++j )
  {
    for ( int k = 0; k <= r; ++k )
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
unsigned greater_or_equal_sinz( S& solver, const clause_t& x, unsigned r, int sid, int spol = 1 )
{
  using boost::adaptors::transformed;

  clause_t xn;
  boost::push_back( xn, x | transformed( []( int v ) { return -v; } ) );
  return less_or_equal_sinz( solver, xn, x.size() - r, sid, spol );
}

template < class S >
unsigned equals_sinz( S& solver, const clause_t& x, unsigned r, int sid )
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

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
