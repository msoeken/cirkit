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
