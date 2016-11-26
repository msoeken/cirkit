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
 * @file abcsat.hpp
 *
 * @brief Interface for the abc sat_solver (bsat)
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef ABCSAT_HPP
#define ABCSAT_HPP

#include <memory>
#include <vector>

#include <classical/abc/abc_api.hpp>
#include <classical/sat/sat_solver.hpp>

#include <sat/bsat/satSolver.h>

namespace cirkit
{

struct abc_solver_t
{
  abc_solver_t() : solver( abc::sat_solver_new() ) {}
  ~abc_solver_t() { abc::sat_solver_delete( solver ); }

  abc::sat_solver * solver;
  std::vector<int>  blocking_vars;
  bool              genmodel = true;
};

using abc_solver = std::unique_ptr<abc_solver_t>;

struct abc_clause_adder
{
  explicit abc_clause_adder( abc_solver& solver ) : solver( solver ) {}

  template<typename C>
  bool add( const C& clause )
  {
    int var;
    std::vector<int> lits = solver->blocking_vars;

    for ( auto parsed_lit : clause )
    {
      var = abs( parsed_lit ) - 1;
      lits.push_back( ( var << 1u ) | ( parsed_lit < 0 ) );
    }

    return sat_solver_addclause( solver->solver, &lits[0], &lits[0] + lits.size() );
  }

private:
  abc_solver& solver;
};

template<>
class solver_traits<abc_solver>
{
public:
  using clause_adder = base_clause_adder<abc_clause_adder>;
};

template<>
abc_solver make_solver<abc_solver>( properties::ptr settings );

template<>
solver_traits<abc_solver>::clause_adder add_clause( abc_solver& solver );

template<>
solver_result_t solve<abc_solver>( abc_solver& solver, solver_execution_statistics& statistics, const std::vector<int>& assumptions );

template<>
void solver_gen_model<abc_solver>( abc_solver& solver, bool genmodel );

template<>
void solver_add_blocking_var( abc_solver& solver, int var );

template<>
void solver_clear_blocking_vars( abc_solver& solver );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
