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

#include "qec.hpp"

#include <cmath>
#include <iostream>

#include <boost/program_options.hpp>

#include <xtensor/xio.hpp>
#include <xtensor/xmath.hpp>

#include <reversible/cli/stores.hpp>
#include <reversible/utils/matrix_utils.hpp>

namespace cirkit
{

using boost::program_options::value;

qec_command::qec_command( const environment::ptr& env )
  : cirkit_command( env, "Quantum equivalence checking" )
{
  opts.add_options()
    ( "qid,q", value( &qids )->composing(), "value of a quantum circuit" )
    ( "rid,r", value( &rids )->composing(), "value of a reversible circuit" )
    ;
}

command::rules_t qec_command::validity_rules() const
{
  return {
    {[this]() { return qids.size() + rids.size() == 2u; }, "two circuits must be selected"},
    {
      [this]() {
        const auto& circuits = env->store<circuit>();
        for ( auto id : qids )
        {
          if ( id >= circuits.size() ) return false;
        }
        for ( auto id : rids )
        {
          if ( id >= circuits.size() ) return false;
        }

        return true;
      },
      "invalid circuit id"
    }
  };
}

bool qec_command::execute()
{
  const auto& circuits = env->store<circuit>();

  std::vector<xt::xarray<complex_t>> matrices;

  for ( auto id : qids )
  {
    matrices.push_back( matrix_from_clifford_t_circuit( circuits[id] ) );
  }
  for ( auto id : rids )
  {
    matrices.push_back( matrix_from_reversible_circuit( circuits[id] ) );
  }

  assert( matrices.size() == 2u );

  result = complex_allclose( matrices[0u], matrices[1u] );
  if ( result )
  {
    std::cout << "[i] circuits are \033[1;32mequivalent\033[0m" << std::endl;
  }
  else
  {
    std::cout << "[i] circuits are \033[1;31mnot equivalent\033[0m" << std::endl;
  }

  return true;
}

command::log_opt_t qec_command::log() const
{
  return log_map_t( {
      {"result", result}
    } );
}


}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
