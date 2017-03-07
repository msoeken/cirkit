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
  const auto conf_budget = get(settings, "conf_budget", -1 );
  //  minisat_solver solver( new Minisat::Solver );
  return { std::move( solver ), std::vector<int>(), true, conf_budget };
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

    /* budget */
    auto limited = false;
    if ( solver.conf_budget > 0 )
    {
      limited = true;
      solver.solver->setConfBudget( solver.conf_budget );
    }
    else
    {
      solver.solver->budgetOff();
    }

    if ( assumptions.empty() )
    {
      if ( limited )
      {
        Minisat::vec<Minisat::Lit> dummy;
        result = ( solver.solver->solveLimited( dummy ) == Minisat::lbool((uint8_t)0) );
      }
      else
      {
        result = solver.solver->solve();
      }
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
      result = limited ? ( solver.solver->solveLimited( lits ) == Minisat::lbool((uint8_t)0) ) : solver.solver->solve( lits );
    }
  }

  statistics.num_vars      = solver.solver->nVars();
  statistics.num_clauses   = solver.solver->nClauses();
  statistics.num_conflicts = solver.solver->conflicts;

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

template<>
void solver_add_blocking_var( minisat_solver& solver, int var )
{
  solver.blocking_vars.push_back( var );
}

template<>
void solver_clear_blocking_vars( minisat_solver& solver )
{
  solver.blocking_vars.clear();
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
