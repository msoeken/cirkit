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

#include "circuit_matrix.hpp"

#include <xtensor/xio.hpp>
#include <xtensor/xmath.hpp>

#include <alice/rules.hpp>
#include <reversible/cli/stores.hpp>
#include <reversible/utils/matrix_utils.hpp>

namespace cirkit
{

circuit_matrix_command::circuit_matrix_command( const environment::ptr& env )
  : cirkit_command( env, "Prints circuit matrix" )
{
  opts.add_options()
    ( "reversible,r", "use reversible simulator" )
    ( "force,f",      "force printing matrix (for circuits with more than 10 variables)" )
    ( "round",        "round matrix before printing" )
    ;
}

command::rules_t circuit_matrix_command::validity_rules() const
{
  return {has_store_element<circuit>( env )};
}

bool circuit_matrix_command::execute()
{
  const auto& circuits = env->store<circuit>();
  const auto& circ = circuits.current();

  if ( circ.lines() > 10u && !is_set( "force" ) )
  {
    std::cout << "[w] circuit has too many variables, use -f to bypass this check" << std::endl;
    return true;
  }

  xt::xarray<complex_t> matrix;

  if ( is_set( "reversible" ) )
  {
    matrix = matrix_from_reversible_circuit( circ );
  }
  else
  {
    matrix = matrix_from_clifford_t_circuit( circ );
  }

  if ( is_set( "round" ) )
  {
    matrix = complex_round( matrix );
  }

  std::cout << matrix << std::endl;

  return true;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
