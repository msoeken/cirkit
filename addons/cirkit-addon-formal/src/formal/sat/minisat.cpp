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

#include "minisat.hpp"

#include <core/utils/range_utils.hpp>
#include <core/utils/timer.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

template<>
minisat_solver make_solver<minisat_solver>( properties::ptr settings )
{
  std::unique_ptr<Minisat::Solver> solver( new Minisat::Solver );
  //  minisat_solver solver( new Minisat::Solver );
  return { std::move( solver ), std::vector<int>(), true };
}

template<>
solver_traits<minisat_solver>::clause_adder add_clause( minisat_solver& solver )
{
  return solver_traits<minisat_solver>::clause_adder( solver );
}

template<>
solver_result_t solve<minisat_solver>( minisat_solver& solver, solver_execution_statistics& statistics, const std::vector<int>& assumptions )
{
  auto result = false;

  {
    reference_timer t( &statistics.runtime );

    if ( assumptions.empty() )
    {
      result = solver.solver->solve();
    }
    else
    {
      const auto nvars = solver.solver->nVars();
      int var;
      Minisat::vec<Minisat::Lit> lits( assumptions.size() );

      for ( auto parsed_lit : index( assumptions ) )
      {
        var = abs( parsed_lit.value ) - 1;
        if ( var >= nvars ) { continue; }
        lits[parsed_lit.index] = ( parsed_lit.value > 0 ) ? Minisat::mkLit( var ) : ~Minisat::mkLit( var );
      }
      result = solver.solver->solve( lits );
    }
  }

  statistics.num_vars    = solver.solver->nVars();
  statistics.num_clauses = solver.solver->nClauses();

  if ( result && !solver.genmodel )
  {
    return solver_result_t( {boost::dynamic_bitset<>(), boost::dynamic_bitset<>(), } );
  }
  else if ( result )
  {
    boost::dynamic_bitset<> bits( solver.solver->nVars() );
    boost::dynamic_bitset<> care( solver.solver->nVars() );

    for ( auto i = 0; i < solver.solver->nVars(); ++i )
    {
      using Minisat::lbool; /* because of the macro in SolverTypes */

      auto v = solver.solver->modelValue( i );
      bits[i] = ( v == l_True );
      care[i] = ( v != l_Undef );
    }

    return solver_result_t( {bits, care} );
  }
  else
  {
    return solver_result_t();
  }
}

template<>
void solver_gen_model<minisat_solver>( minisat_solver& solver, bool genmodel )
{
  solver.genmodel = genmodel;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
