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

#include <classical/sat/sat_solver.hpp>

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
void blocking_and( S& solver, int sel, const clause_t& x, int c )
{
  using boost::adaptors::transformed;

  for ( auto l : x )
  {
    add_clause( solver )( {-sel, l, -c} );
  }
  clause_t clause = { -sel };
  boost::push_back( clause, x | transformed( []( int l ) { return -l; } ) );
  clause += c;
  add_clause( solver )( clause );
}

template<class S>
inline void blocking_or( S& solver, int sel, int a, int b, int c )
{
  add_clause( solver )( {-sel, -a, c} );
  add_clause( solver )( {-sel, -b, c} );
  add_clause( solver )( {-sel, a, b, -c} );
}

template<class S>
void blocking_or( S& solver, int sel, const clause_t& x, int c )
{
  for ( auto l : x )
  {
    add_clause( solver )( {-sel, -l, c} );
  }
  clause_t clause( x.begin(), x.end() );
  clause += -sel;
  clause += -c;
  add_clause( solver )( clause );
}

template<class S>
inline void blocking_xor( S& solver, int sel, int a, int b, int c )
{
  add_clause( solver )( {-sel, -a, b, c} );
  add_clause( solver )( {-sel, a, -b, c} );
  add_clause( solver )( {-sel, a, b, -c} );
  add_clause( solver )( {-sel, -a, -b, -c} );
}

template<class S>
inline void blocking_xnor( S& solver, int sel, int a, int b, int c )
{
  add_clause( solver )( {-sel, -a, b, -c} );
  add_clause( solver )( {-sel, a, -b, -c} );
  add_clause( solver )( {-sel, a, b, c} );
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
