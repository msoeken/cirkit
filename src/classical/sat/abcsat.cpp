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
  std::unique_ptr<abc_solver_t> solver( new abc_solver_t );
  return solver;
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
      result = abc::sat_solver_solve( solver->solver, nullptr, nullptr, 0, 0, 0, 0 );
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

      result = abc::sat_solver_solve( solver->solver, &lits[0], &lits[0] + lits.size(), 0, 0, 0, 0 );
    }
  }

  statistics.num_vars    = abc::sat_solver_nvars( solver->solver );
  statistics.num_clauses = abc::sat_solver_nclauses( solver->solver );

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
      auto v = sat_solver_get_var_value( solver->solver, i );
      bits[i] = ( v == abc::l_True );
      care[i] = ( v != abc::l_Undef );
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
