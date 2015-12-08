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

/**
 * @file logic.hpp
 *
 * @brief Logic operations
 *
 * @author Mathias Soeken
 * @since  2.2
 */

#ifndef LOGIC_HPP
#define LOGIC_HPP

#include <boost/assign/std/vector.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>

#include <formal/sat/sat_solver.hpp>

using namespace boost::assign;

namespace cirkit
{

template<class S>
inline void blocking_and( S& solver, int sel, int a, int b, int c )
{
  add_clause( solver )( {-sel, a, -c} );
  add_clause( solver )( {-sel, b, -c} );
  add_clause( solver )( {-sel, -a, -b, c} );
}

template<class S>
inline void logic_and( S& solver, int a, int b, int c )
{
  add_clause( solver )( {a, -c} );
  add_clause( solver )( {b, -c} );
  add_clause( solver )( {-a, -b, c} );
}

template<class S>
void logic_and( S& solver, const clause_t& x, int c )
{
  using boost::adaptors::transformed;

  for ( auto l : x )
  {
    add_clause( solver )( {l, -c} );
  }
  clause_t clause;
  boost::push_back( clause, x | transformed( []( int l ) { return -l; } ) );
  clause += c;
  add_clause( solver )( clause );
}

template<class S>
inline void logic_or( S& solver, int a, int b, int c )
{
  add_clause( solver )( {-a, c} );
  add_clause( solver )( {-b, c} );
  add_clause( solver )( {a, b, -c} );
}

template<class S>
void logic_or( S& solver, const clause_t& x, int c )
{
  for ( auto l : x )
  {
    add_clause( solver )( {-l, c} );
  }
  clause_t clause( x.begin(), x.end() );
  clause += -c;
  add_clause( solver )( clause );
}

template<class S>
inline void logic_xor( S& solver, int a, int b, int c )
{
  add_clause( solver )( {-a, b, c} );
  add_clause( solver )( {a, -b, c} );
  add_clause( solver )( {a, b, -c} );
  add_clause( solver )( {-a, -b, -c} );
}

template<class S>
inline void logic_xnor( S& solver, int a, int b, int c )
{
  add_clause( solver )( {-a, b, -c} );
  add_clause( solver )( {a, -b, -c} );
  add_clause( solver )( {a, b, c} );
  add_clause( solver )( {-a, -b, c} );
}

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
