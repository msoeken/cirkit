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
 * @file depqbf.hpp
 *
 * @brief QBF solver interface for DepQBF
 *
 * @author Heinz Riener
 */

#ifndef DEPQBF_HPP
#define DEPQBF_HPP

#include <classical/sat/qbf_solver.hpp>
#include <boost/assign/std/vector.hpp>

extern "C"
{
#include <qdpll.h>
}

namespace cirkit
{
  
namespace detail
{
struct depqbf_wrapper
{
  depqbf_wrapper()
    : qdpll( qdpll_create() )
  {
    /* Use the linear ordering of the quantifier prefix. */
    qdpll_configure(qdpll, "--dep-man=simple");
    /* Enable incremental solving. */
    qdpll_configure(qdpll, "--incremental-use");
  }

  ~depqbf_wrapper()
  {
    qdpll_delete( qdpll );
    qdpll = nullptr;
  }

  void add( LitID lit )
  {
    if ( !ready ) { reset(); }
    qdpll_add( qdpll, lit );
  }

  QDPLLResult solve()
  {
    if ( !ready ) { reset(); }
    ready = false;
    return qdpll_sat( qdpll );
  }

  void new_scope( QDPLLQuantifierType qtype )
  {
    qdpll_new_scope( qdpll, qtype );
  }

  void new_scope( QDPLLQuantifierType qtype, unsigned nesting )
  {
    assert( nesting > 0 );
    qdpll_new_scope_at_nesting( qdpll, qtype, nesting );
  }
  
  QDPLLAssignment get_value( VarID i )
  {
    return qdpll_get_value( qdpll, i );
  }

private:
  void reset()
  {
    qdpll_reset( qdpll );
  }

  QDPLL *qdpll;
  bool ready = true;
}; /* depqbf_wrapper */
}

struct depqbf_solver
{
  std::unique_ptr<detail::depqbf_wrapper> solver;
  std::vector< LitID > lits;
  unsigned max_id;
};

struct depqbf_clause_adder
{
  explicit depqbf_clause_adder( depqbf_solver& solver ) : solver( solver ) {}

  template< typename C >
  bool add( const C& clause )
  {
    using namespace boost::assign;
    for ( const auto& lit : clause )
    {
      if ( abs( lit ) > solver.max_id ) { solver.max_id = abs( lit ); }
      solver.lits += lit;
    }
    solver.lits += 0;
    return true;
  }

private:
  depqbf_solver& solver;
}; /* depqbf_clause_adder */

template<>
class solver_traits<depqbf_solver>
{
public:
  using clause_adder = base_clause_adder<depqbf_clause_adder>;
}; /* solver_traits */

template<>
depqbf_solver make_solver<depqbf_solver>( properties::ptr settings );

template<>
solver_traits<depqbf_solver>::clause_adder add_clause( depqbf_solver& solver );

template<>
solver_result_t solve<depqbf_solver>(depqbf_solver& solver, solver_execution_statistics& statistics, const std::vector<int>& assumptions);  

void new_scope(depqbf_solver& solver, quantifier_t type, const std::vector<int>& vars);
void add_clauses_to_scope(depqbf_solver& solver);

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
