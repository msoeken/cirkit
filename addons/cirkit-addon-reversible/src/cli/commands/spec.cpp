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

#include "spec.hpp"

#include <cmath>
#include <vector>

#include <alice/rules.hpp>

#include <core/utils/string_utils.hpp>
#include <reversible/circuit.hpp>
#include <reversible/truth_table.hpp>
#include <cli/reversible_stores.hpp>
#include <reversible/functions/circuit_to_truth_table.hpp>
#include <reversible/functions/permutation_to_truth_table.hpp>
#include <reversible/simulation/simple_simulation.hpp>

namespace cirkit
{

spec_command::spec_command( const environment::ptr& env )
  : cirkit_command( env, "Specification functions" )
{
  add_flag( "--circuit,-c", "read from current circuit" );
  add_option( "--permutation,-p", permutation, "create spec from permutation (starts with 0, space separated)" );
  add_new_option();
}

command::rules spec_command::validity_rules() const
{
  return {
    { [this]() { return is_set( "circuit" ) != is_set( "permutation" ); }, "either circuit or permutation must be set" },
    { [this]() { return !is_set( "circuit" ) || env->store<circuit>().current_index() >= 0; }, "no circuit in store" }
  };
}

void spec_command::execute()
{
  auto& specs = env->store<binary_truth_table>();

  if ( specs.empty() || is_set( "new" ) )
  {
    specs.extend();
  }

  if ( is_set( "circuit" ) )
  {
    auto& circuits = env->store<circuit>();

    const auto& circ = circuits.current();

    binary_truth_table spec;
    circuit_to_truth_table( circ, spec, simple_simulation_func() );

    specs.current() = spec;
  }
  else if ( is_set( "permutation" ) )
  {
    std::vector<unsigned> perm;
    parse_string_list( perm, permutation );

    specs.current() =  permutation_to_truth_table( perm );
  }
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
