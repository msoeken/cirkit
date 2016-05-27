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
  unsigned num_vars;
  unsigned num_clauses;
  double parse_time;
  double runtime;
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
