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
 * @file sat_solver.hpp
 *
 * @brief Generic SAT solver interface
 *
 * @author Mathias Soeken
 * @author Heinz Riener
 *
 * @since  2.2
 */

/******************************************************************************
 * Simple generic SAT solver interface                                        *
 ******************************************************************************
 *                                                                            *
 * Synopsis:                                                                  *
 *                                                                            *
 *   Provides some simple generic functions which are used in many SAT-based  *
 *   algorithms.  These functions can be specialized for several SAT solvers. *
 *                                                                            *
 * API:                                                                       *
 *                                                                            *
 *   Solver make_solver( [properties] )                                       *
 *     Creates a solver                                                       *
 *                                                                            *
 *   bool add_clause( solver )( clause )                                      *
 *     Adds a clause (container of literals) to the solver and returns false, *
 *     if a conflict was detected and the instance is UNSAT.                  *
 *     Each literal is a positive or negative integer different from 0.       *
 *                                                                            *
 *   solver_result_t solve( solver, [statistics, [assumptions]] )             *
 *     Solves the instance.  The statistics data stores runtime, number of    *
 *     variables and number of clauses. Assumptions are a list of literals.   *
 *                                                                            *
 *   solver_gen_model( solver, genmodel )                                     *
 *     Allows to turn off model generation, if one is only interested in the  *
 *     SAT/UNSAT answer.                                                      *
 ******************************************************************************/

#ifndef SAT_SOLVER_HPP
#define SAT_SOLVER_HPP

#include <vector>

#include <boost/dynamic_bitset.hpp>
#include <boost/optional.hpp>

#include <core/properties.hpp>

namespace cirkit
{

using clause_t = std::vector<int>;

/**
 * @brief Solver result type
 *
 * If the value is empty, the result was unsatisfiable, otherwise the first
 * bitset holds the values of assigned variables where the second bitset masks
 * the assigned variables (1 = contained, 0 = don't care)
 */
using solver_result_t = boost::optional<std::pair<boost::dynamic_bitset<>, boost::dynamic_bitset<>>>;

struct solver_execution_statistics
{
  unsigned num_vars = 0;
  unsigned num_clauses = 0;
  double parse_time = 0.0;
  double runtime = 0.0;
  uint64_t num_conflicts = 0;
};

template<class S>
class solver_traits {};

template<class Base>
class base_clause_adder : public Base
{
public:
  template<class S>
  base_clause_adder( S& solver ) : Base( solver ) {}

  template<class C>
  inline bool operator()( const C& clause )
  {
    return Base::add( clause );
  }

  inline bool operator()( const std::initializer_list<int>& clause )
  {
    return Base::add( clause );
  }
};

template<class S>
S make_solver( properties::ptr settings = properties::ptr() );

template<class S>
typename solver_traits<S>::clause_adder add_clause( S& solver );

template<class S>
solver_result_t solve( S& solver, solver_execution_statistics& statistics, const std::vector<int>& assumptions = std::vector<int>() );

template<class S>
solver_result_t solve( S& solver )
{
  solver_execution_statistics ignore;
  return solve( solver, ignore );
}

template<class S>
void solver_gen_model( S& solver, bool genmodel )
{
  assert( false && "not implemented" );
}

template<class S>
void solver_add_blocking_var( S& solver, int var )
{
  assert( false && "not implemented" );
}

template<class S>
void solver_clear_blocking_vars( S& solver )
{
  assert( false && "not implemented" );
}

template<class S>
inline void equals( S& solver, int a, int b )
{
  add_clause( solver )( {-a, b} );
  add_clause( solver )( {a, -b} );
}

template<class S>
inline void not_equals( S& solver, int a, int b )
{
  add_clause( solver )( {a, b} );
  add_clause( solver )( {-a, -b} );
}

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
