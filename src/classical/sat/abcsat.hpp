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
 * @file abcsat.hpp
 *
 * @brief Interface for the abc sat_solver (bsat)
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef ABCSAT_HPP
#define ABCSAT_HPP

#include <memory>
#include <vector>

#include <classical/abc/abc_api.hpp>
#include <classical/sat/sat_solver.hpp>

#include <sat/bsat/satSolver.h>
#include <sat/bsat/satVec.h>

namespace cirkit
{

struct abc_solver_t
{
  abc_solver_t() : solver( abc::sat_solver_new() ) {}
  abc_solver_t( abc::sat_solver * other ) : solver( other ), borrowed( true ) {}
  ~abc_solver_t()
  {
    if ( !borrowed )
    {
      abc::sat_solver_delete( solver );
    }
  }

  abc::sat_solver * solver;
  std::vector<int>  blocking_vars;
  bool              genmodel = true;
  bool              borrowed = false;
  int               conf_budget = 0; /* 0 : unlimited */
};

using abc_solver = std::unique_ptr<abc_solver_t>;

struct abc_clause_adder
{
  explicit abc_clause_adder( abc_solver& solver ) : solver( solver ) {}

  template<typename C>
  bool add( const C& clause )
  {
    int var;
    std::vector<int> lits = solver->blocking_vars;

    for ( auto parsed_lit : clause )
    {
      var = abs( parsed_lit ) - 1;
      lits.push_back( ( var << 1u ) | ( parsed_lit < 0 ) );
    }

    return sat_solver_addclause( solver->solver, &lits[0], &lits[0] + lits.size() );
  }

private:
  abc_solver& solver;
};

template<>
class solver_traits<abc_solver>
{
public:
  using clause_adder = base_clause_adder<abc_clause_adder>;
};

template<>
abc_solver make_solver<abc_solver>( properties::ptr settings );

template<>
solver_traits<abc_solver>::clause_adder add_clause( abc_solver& solver );

template<>
solver_result_t solve<abc_solver>( abc_solver& solver, solver_execution_statistics& statistics, const std::vector<int>& assumptions );

template<>
void solver_gen_model<abc_solver>( abc_solver& solver, bool genmodel );

template<>
void solver_add_blocking_var( abc_solver& solver, int var );

template<>
void solver_clear_blocking_vars( abc_solver& solver );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
