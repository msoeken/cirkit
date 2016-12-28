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

#include "depqbf.hpp"

namespace cirkit
{

template<>
depqbf_solver make_solver<depqbf_solver>( properties::ptr settings )
{
  std::unique_ptr<detail::depqbf_wrapper> solver( new detail::depqbf_wrapper );
  return { std::move( solver ), {}, 0u };
}

template<>
solver_traits<depqbf_solver>::clause_adder add_clause( depqbf_solver& solver )
{
  return solver_traits<depqbf_solver>::clause_adder( solver );
}

template<>
solver_result_t solve<depqbf_solver>( depqbf_solver& solver, solver_execution_statistics& statistics, const std::vector<int>& assumptions )
{
  const auto result = solver.solver->solve();
  if ( result == QDPLL_RESULT_UNSAT )
  {
    return solver_result_t();
  }

  assert( result == QDPLL_RESULT_SAT );
  boost::dynamic_bitset<> bits( solver.max_id - 1u ), care( solver.max_id - 1u );
  for ( auto i = 1; i < solver.max_id; ++i )
  {
    const QDPLLAssignment a = solver.solver->get_value( i );
    bits[ i-1 ] = ( a == QDPLL_ASSIGNMENT_FALSE ? 0 : 1 );
    care[ i-1 ] = ( a == QDPLL_ASSIGNMENT_UNDEF ? 0 : 1 );
  }
  return solver_result_t( { bits, care } );
}

void new_scope(depqbf_solver& solver, quantifier_t type, const std::vector<int>& vars)
{
  QDPLLQuantifierType qdpll_type;
  switch ( type )
  {
  case quantifier_t::UNDEF:
    qdpll_type = QDPLL_QTYPE_UNDEF;    
    break;
  case quantifier_t::EXISTS:
    qdpll_type = QDPLL_QTYPE_EXISTS;
    break;
  case quantifier_t::FORALL:
    qdpll_type = QDPLL_QTYPE_FORALL;
    break;
  default:
    assert( false );
  };
  solver.solver->new_scope( qdpll_type );
  for ( const auto& v : vars )
  {
    solver.solver->add( v );
  }
  solver.solver->add( 0 );
}

void add_clauses_to_scope(depqbf_solver& solver)
{
  for ( const auto& lit : solver.lits )
  {
    solver.solver->add( lit );
  }
  solver.lits.clear();
}

}
// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
