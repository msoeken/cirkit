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

#include "cryptominisat.hpp"

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
cryptominisat_solver make_solver<cryptominisat_solver>( properties::ptr settings )
{
  std::unique_ptr<CMSat::SATSolver> solver( new CMSat::SATSolver );
  solver->set_num_threads( 4 );
  return { std::move( solver ), std::vector<int>(), true, 0, 0, -1 };
}

template<>
solver_traits<cryptominisat_solver>::clause_adder add_clause( cryptominisat_solver& solver )
{
  return solver_traits<cryptominisat_solver>::clause_adder( solver );
}

solver_traits<cryptominisat_solver>::xor_clause_adder add_xor_clause( cryptominisat_solver& solver )
{
  return solver_traits<cryptominisat_solver>::xor_clause_adder( solver );
}

template<>
solver_result_t solve<cryptominisat_solver>( cryptominisat_solver& solver, solver_execution_statistics& statistics, const std::vector<int>& assumptions )
{
  /* budget */
  if ( solver.conf_budget > 0 )
  {
    assert( false && "cryptominisat has no support for budget configurations" );
  }

  {
    reference_timer t( &statistics.runtime );

    const auto nvars = solver.solver->nVars();
    int var;
    std::vector<CMSat::Lit> lits( assumptions.size() );
    for ( auto parsed_lit : index( assumptions ) )
    {
      var = abs( parsed_lit.value ) - 1;
      if ( var >= int(nvars) ) { continue; }
      lits[parsed_lit.index] = ( parsed_lit.value > 0 ) ? CMSat::Lit( var, false ) : CMSat::Lit( var, true );
    }

    auto result = solver.solver->solve( &lits );

    statistics.num_vars    = solver.solver->nVars();
    statistics.num_clauses = solver.num_clauses + solver.num_x_clauses;

    using CMSat::lbool;
    if ( result == l_True && !solver.genmodel )
    {
      return solver_result_t( {boost::dynamic_bitset<>(), boost::dynamic_bitset<>(), } );
    }
    else if ( result == l_True )
    {
      boost::dynamic_bitset<> bits( solver.solver->nVars() );
      boost::dynamic_bitset<> care( solver.solver->nVars() );

      auto v = solver.solver->get_model();
      for ( auto i = 0u; i < solver.solver->nVars(); ++i )
      {

        bits[i] = ( v[i] == l_True );
        care[i] = ( v[i] != l_Undef );
      }

      return solver_result_t( { bits, care } );
    }
    else
    {
      return solver_result_t();
    }
  }
}

template<>
void solver_gen_model<cryptominisat_solver>( cryptominisat_solver& solver, bool genmodel )
{
  solver.genmodel = genmodel;
}

template<>
void solver_add_blocking_var( cryptominisat_solver& solver, int var )
{
  solver.blocking_vars.push_back( var );
}

template<>
void solver_clear_blocking_vars( cryptominisat_solver& solver )
{
  solver.blocking_vars.clear();
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
