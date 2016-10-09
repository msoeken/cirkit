/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2016  EPFL
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
 * @file cryptominisat.hpp
 *
 * @brief Generic SAT solver implementation based on CryptoMiniSat
 *
 * @author Heinz Riener
 * @since  2.3
 */

#ifndef CRYPTOMINISAT_HPP
#define CRYPTOMINISAT_HPP

#include <classical/sat/sat_solver.hpp>
#include <cryptominisat5/cryptominisat.h>

namespace cirkit
{

struct cryptominisat_solver
{
  std::unique_ptr<CMSat::SATSolver> solver;
  std::vector<int>                  blocking_vars;
  bool                              genmodel;
  unsigned                          num_clauses;
  unsigned                          num_x_clauses;
  int                               conf_budget; /* -1 : unlimited */
};

struct cryptominisat_clause_adder
{
  explicit cryptominisat_clause_adder( cryptominisat_solver& solver ) : solver( solver ) {}

  void resize( unsigned max_var )
  {
    for ( auto x = solver.solver->nVars(); x <= max_var; ++x )
    {
      solver.solver->new_var();
    }
  }
  
  template <typename Clause>
  void add_to_lits( std::vector<CMSat::Lit>& lits, const Clause& clause )
  {
    int var;
    for ( auto lit : clause )
    {
      var = abs(lit) - 1;
      resize( var );
      lits.push_back( ( lit > 0 ) ? CMSat::Lit( var, false ) : CMSat::Lit( var, true ) );
    }
  }

  template <typename Clause>
  bool add( const Clause& clause )
  {
    std::vector<CMSat::Lit> lits;
    add_to_lits( lits, solver.blocking_vars );
    add_to_lits( lits, clause );
    solver.num_clauses += 1;
    return solver.solver->add_clause(lits);
  }

private:
  cryptominisat_solver& solver;
}; /* cryptominisat_clause_adder */

struct cryptominisat_xor_clause_adder
{
  explicit cryptominisat_xor_clause_adder( cryptominisat_solver& solver ) : solver( solver ) {}

  template<class C>
  inline bool operator()( const C& clause, bool complement = false )
  {
    return add( clause, complement );
  }

  inline bool operator()( const std::initializer_list<int>& clause, bool complement = false )
  {
    return add( clause, complement );
  }

  void resize( unsigned max_var )
  {
    for ( auto x = solver.solver->nVars(); x <= max_var; ++x )
    {
      solver.solver->new_var();
    }
  }
  
  template <typename Clause>
  void add_to_lits( std::vector<unsigned>& vars, const Clause& clause )
  {
    for ( auto var : clause )
    {
      auto v = var - 1;
      resize( v );
      vars.push_back( v );
    }
  }

  template <typename Clause>
  bool add( const Clause& clause, bool rhs )
  {
    std::vector<unsigned> vars;
    add_to_lits( vars, clause );
    solver.num_x_clauses += 1;
    return solver.solver->add_xor_clause(vars,!rhs);
  }

private:
  cryptominisat_solver& solver;
}; /* cryptominisat_xor_clause_adder */

template<>
class solver_traits<cryptominisat_solver>
{
public:
  using clause_adder = base_clause_adder<cryptominisat_clause_adder>;
  using xor_clause_adder = cryptominisat_xor_clause_adder;
};

template<>
cryptominisat_solver make_solver<cryptominisat_solver>( properties::ptr settings );

template<>
solver_traits<cryptominisat_solver>::clause_adder add_clause( cryptominisat_solver& solver );

solver_traits<cryptominisat_solver>::xor_clause_adder add_xor_clause( cryptominisat_solver& solver );

template<>
solver_result_t solve<cryptominisat_solver>( cryptominisat_solver& solver, solver_execution_statistics& statistics, const std::vector<int>& assumptions );

template<>
void solver_gen_model<cryptominisat_solver>( cryptominisat_solver& solver, bool genmodel );
  
} /* cirkit */

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
