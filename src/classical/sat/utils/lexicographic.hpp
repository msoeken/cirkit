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
 * @file lexicographic.hpp
 *
 * @brief Find lexicographically smallest and largest solutions
 *
 * Based on Knuth TAOCP Exercise 7.2.2.2-109.
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef LEXICOGRAPHIC_SAT_HPP
#define LEXICOGRAPHIC_SAT_HPP

#include <exception>
#include <iostream>
#include <vector>

#include <boost/dynamic_bitset.hpp>

#include <core/utils/range_utils.hpp>
#include <classical/sat/sat_solver.hpp>
#include <classical/sat/utils/visit_solutions.hpp>

namespace cirkit
{

class unsat_exception : public std::exception
{
  virtual const char * what() const throw()
  {
    return "Solution is UNSAT";
  }
};

template <typename Solver>
boost::dynamic_bitset<>
lexicographic_solution( Solver& solver, const std::vector<int>& vars,
                        const std::vector<int>& assumptions, bool smallest,
                        const properties::ptr& settings,
                        const properties::ptr& statistics )
{
  auto runtime   = 0.0;
  auto sat_calls = 0u;
  auto clauses   = 0u;

  /* F1. [Initialize.] */
  solver_execution_statistics stats;
  auto result = solve( solver, stats, assumptions );
  runtime += stats.runtime;
  clauses = stats.num_clauses;
  ++sat_calls;

  if ( result == boost::none )
  {
    throw unsat_exception();
  }

  auto ys = extract_solution( result, vars );

  int d_iter = -1;

  while ( true )
  {
    /* F2. [Advance d.] */
    std::string s;
    to_string( ys, s );
    if ( smallest )
    {
      d_iter = s.find( "1", d_iter + 1 );
    }
    else
    {
      d_iter = s.find( "0", d_iter + 1 );
    }

    /* F3. [Done?] */
    if ( d_iter == std::string::npos )
    {
      set( statistics, "runtime", runtime );
      set( statistics, "sat_calls", sat_calls );
      set( statistics, "num_clauses", clauses );
      return ys;
    }

    unsigned d = s.size() - d_iter - 1;

    /* F4. [Try for smaller/larger.] */
    auto local_assumptions = assumptions;
    for ( auto j = vars.size() - 1; j > d; --j )
    {
      local_assumptions.push_back( ys[j] ? vars[j] : -vars[j] );
    }
    local_assumptions.push_back( smallest ? -vars[d] : vars[d] );

    result = solve( solver, stats, local_assumptions );
    runtime += stats.runtime;
    ++sat_calls;
    if ( result != boost::none )
    {
      ys = extract_solution( result, vars );
    }
  }
}

template<typename Solver>
boost::dynamic_bitset<> lexicographic_smallest_solution( Solver& solver, const std::vector<int>& vars,
                                                         const std::vector<int>& assumptions = std::vector<int>(),
                                                         const properties::ptr& settings = properties::ptr(),
                                                         const properties::ptr& statistics = properties::ptr() )
{
  return lexicographic_solution( solver, vars, assumptions, true, settings, statistics );
}

template<typename Solver>
boost::dynamic_bitset<> lexicographic_largest_solution( Solver& solver, const std::vector<int>& vars,
                                                        const std::vector<int>& assumptions = std::vector<int>(),
                                                        const properties::ptr& settings = properties::ptr(),
                                                        const properties::ptr& statistics = properties::ptr() )
{
  return lexicographic_solution( solver, vars, assumptions, false, settings, statistics );
}

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
