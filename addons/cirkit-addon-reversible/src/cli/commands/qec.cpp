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

#include <xtensor/xio.hpp>
#include <xtensor/xmath.hpp>
#include <xtensor-blas/xlinalg.hpp>

#include <cli/reversible_stores.hpp>
#include <reversible/utils/matrix_utils.hpp>

namespace cirkit
{

qec_command::qec_command( const environment::ptr& env )
  : cirkit_command( env, "Quantum equivalence checking" )
{
  add_option( "--qid,-q", qids, "value of a quantum circuit" );
  add_option( "--rid,-r", rids, "value of a reversible circuit" );
  add_flag( "--ancilla,-a", "add ancilla (to the end) if necessary" );
  add_flag( "--progress,-p", "show progress" );
  add_flag( "--quiet", "do not print result" );
}

command::rules qec_command::validity_rules() const
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

void qec_command::execute()
{
  const auto& circuits = env->store<circuit>();

  std::vector<std::pair<xt::xarray<complex_t>, unsigned>> matrices;

  for ( auto id : qids )
  {
    matrices.push_back( {matrix_from_clifford_t_circuit( circuits[id], is_set( "progress" ) ), circuits[id].lines()} );
  }
  for ( auto id : rids )
  {
    matrices.push_back( {matrix_from_reversible_circuit( circuits[id] ), circuits[id].lines()} );
  }

  assert( matrices.size() == 2u );

  /* add ancillas if necessary */
  if ( matrices[0u].second != matrices[1u].second )
  {
    if ( is_set( "ancilla" ) )
    {
      const auto d0 = matrices[0u].second;
      const auto d1 = matrices[1u].second;

      if ( d0 < d1 )
      {
        /* make first matrix larger */
        matrices[0u].first = xt::linalg::kron( identity( 1 << ( d1 - d0 ) ), matrices[0u].first );
      }
      else
      {
        /* make first matrix larger */
        matrices[1u].first = xt::linalg::kron( identity( 1 << ( d0 - d1 ) ), matrices[1u].first );
      }
    }
    else
    {
      std::cout << "[e] matrices have different dimensions, use ancilla option to adjust." << std::endl;
      return;
    }
  }

  result = complex_allclose( matrices[0u].first, matrices[1u].first );
  if ( is_set( "quiet" ) ) return;

  if ( result )
  {
    std::cout << "[i] circuits are \033[1;32mequivalent\033[0m" << std::endl;
  }
  else
  {
    std::cout << "[i] circuits are \033[1;31mnot equivalent\033[0m" << std::endl;
  }

  qids.clear();
  rids.clear();
}

nlohmann::json qec_command::log() const
{
  return nlohmann::json( {
      {"result", result}
    } );
}


}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
