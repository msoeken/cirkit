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

#include <classical/sat/sat_solver.hpp>

namespace cirkit
{

struct minisat_solver
{
  std::unique_ptr<Minisat::Solver> solver;
  std::vector<int>                 blocking_vars;
  bool                             genmodel;
  int                              conf_budget; /* -1 : unlimited */
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

template<>
void solver_add_blocking_var( minisat_solver& solver, int var );

template<>
void solver_clear_blocking_vars( minisat_solver& solver );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
