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
 * @file visit_solutions.hpp
 *
 * @brief Visit all SAT solutions
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef VISIT_SOLUTIONS_HPP
#define VISIT_SOLUTIONS_HPP

#include <vector>

#include <boost/dynamic_bitset.hpp>

#include <classical/sat/sat_solver.hpp>

namespace cirkit
{

boost::dynamic_bitset<> extract_solution( const solver_result_t& result, const std::vector<int>& vars );

template<typename Solver, typename Fn>
int foreach_solution( Solver& solver, const std::vector<int>& vars, int sid, const Fn&& f, const std::vector<int>& assumptions = std::vector<int>() )
{
  solver_result_t result;
  std::vector<int> local_assumptions = assumptions;
  solver_execution_statistics stats;

  while ( ( result = solve( solver, stats, local_assumptions ) ) != boost::none )
  {
    const auto solution = extract_solution( result, vars );
    if ( !f( solution ) )
    {
      break;
    }

    std::vector<int> blocking( vars.size() + 1u );
    for ( auto i = 0u; i < vars.size(); ++i )
    {
      blocking[i + 1u] = solution[i] ? -vars[i] : vars[i];
    }
    blocking[0] = sid++;
    add_clause( solver )( blocking );

    local_assumptions.push_back( -blocking[0] );
  }

  return sid;
}

template<typename Solver>
int all_sat( Solver& solver, const std::vector<int>& vars, int sid, boost::dynamic_bitset<>& truth_table, const std::vector<int>& assumptions = std::vector<int>() )
{
  truth_table.resize( 1u << vars.size() );

  return foreach_solution( solver, vars, sid, [&]( const boost::dynamic_bitset<>& solution ) {
      truth_table.set( solution.to_ulong() );
      return true;
    }, assumptions );
}

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
