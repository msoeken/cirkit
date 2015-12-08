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
 * @file minisat.hpp
 *
 * @brief Generic SAT solver implementation based on MiniSAT
 *
 * @author Mathias Soeken
 * @since  2.2
 */

#ifndef MINISAT_HPP
#define MINISAT_HPP

#include <core/Solver.h>

#include <formal/sat/sat_solver.hpp>

namespace cirkit
{

struct minisat_solver
{
  std::unique_ptr<Minisat::Solver> solver;
  std::vector<int>                 blocking_vars;
  bool                             genmodel;
};

struct minisat_clause_adder
{
  explicit minisat_clause_adder( minisat_solver& solver ) : solver( solver ) {}

  void resize( int max_var )
  {
    for ( auto x = solver.solver->nVars(); x <= max_var; ++x )
    {
      solver.solver->newVar();
    }
  }

  template<typename C>
  void add_to_lits( Minisat::vec<Minisat::Lit>& lits, const C& clause )
  {
    int var;

    for ( auto lit : clause )
    {
      var = abs( lit ) - 1;
      resize( var );
      lits.push( ( lit > 0 ) ? Minisat::mkLit( var ) : ~Minisat::mkLit( var ) );
    }
  }

  template<typename C>
  bool add( const C& clause )
  {
    Minisat::vec<Minisat::Lit> lits;

    add_to_lits( lits, solver.blocking_vars );
    add_to_lits( lits, clause );

    return solver.solver->addClause( lits );
  }

private:
  minisat_solver& solver;
};

template<>
class solver_traits<minisat_solver>
{
public:
  using clause_adder = base_clause_adder<minisat_clause_adder>;
};

template<>
minisat_solver make_solver<minisat_solver>( properties::ptr settings );

template<>
solver_traits<minisat_solver>::clause_adder add_clause( minisat_solver& solver );

template<>
solver_result_t solve<minisat_solver>( minisat_solver& solver, solver_execution_statistics& statistics, const std::vector<int>& assumptions );

template<>
void solver_gen_model<minisat_solver>( minisat_solver& solver, bool genmodel );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
