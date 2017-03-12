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

#include "abcsat.hpp"

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
abc_solver make_solver<abc_solver>( properties::ptr settings )
{
  const auto reuse_solver = get<abc::sat_solver*>( settings, "reuse_solver", nullptr );
  const auto conf_budget = get(settings, "conf_budget", 0 );

  if ( reuse_solver )
  {
    std::unique_ptr<abc_solver_t> solver( new abc_solver_t( reuse_solver ) );
    solver->conf_budget = conf_budget;
    return solver;
  }
  else
  {
    std::unique_ptr<abc_solver_t> solver( new abc_solver_t );
    solver->conf_budget = conf_budget;
    return solver;
  }
}

template<>
solver_traits<abc_solver>::clause_adder add_clause( abc_solver& solver )
{
  return solver_traits<abc_solver>::clause_adder( solver );
}

template<>
solver_result_t solve<abc_solver>( abc_solver& solver, solver_execution_statistics& statistics, const std::vector<int>& assumptions )
{
  abc::lbool result = abc::l_Undef;

  {
    reference_timer t( &statistics.runtime );

    if ( assumptions.empty() )
    {
      result = abc::sat_solver_solve( solver->solver, nullptr, nullptr, /* nConfLimit = */solver->conf_budget, 0, 0, 0 );
    }
    else
    {
      int var;
      std::vector<int> lits;
      const auto nvars = abc::sat_solver_nvars( solver->solver );

      for ( auto parsed_lit : assumptions )
      {
        var = abs( parsed_lit ) - 1;

        if ( var >= nvars ) { continue; }

        assert( var < abc::sat_solver_nvars( solver->solver ) );

        lits.push_back( ( var << 1u ) | ( parsed_lit < 0 ) );
      }

      result = abc::sat_solver_solve( solver->solver, &lits[0], &lits[0] + lits.size(), /* nConfLimit = */solver->conf_budget, 0, 0, 0 );
    }
  }

  statistics.num_vars      = abc::sat_solver_nvars( solver->solver );
  statistics.num_clauses   = abc::sat_solver_nclauses( solver->solver );
  statistics.num_conflicts = abc::sat_solver_nconflicts( solver->solver );

  if ( result == abc::l_True && !solver->genmodel )
  {
    return solver_result_t( {boost::dynamic_bitset<>(), boost::dynamic_bitset<>(), } );
  }
  else if ( result == abc::l_True )
  {
    boost::dynamic_bitset<> bits( statistics.num_vars );
    boost::dynamic_bitset<> care( statistics.num_vars );

    for ( auto i = 0u; i < statistics.num_vars; ++i )
    {
      bits[i] = sat_solver_var_value( solver->solver, i ) == 1;
      care[i] = true;
    }

    return solver_result_t( {bits, care} );
  }
  else
  {
    return solver_result_t();
  }
}

template<>
void solver_gen_model<abc_solver>( abc_solver& solver, bool genmodel )
{
  solver->genmodel = genmodel;
}

template<>
void solver_add_blocking_var( abc_solver& solver, int var )
{
  solver->blocking_vars.push_back( var );
}

template<>
void solver_clear_blocking_vars( abc_solver& solver )
{
  solver->blocking_vars.clear();
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
