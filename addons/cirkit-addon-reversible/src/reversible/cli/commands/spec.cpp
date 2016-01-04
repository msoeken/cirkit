/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
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

#include "spec.hpp"

#include <cmath>
#include <vector>

#include <core/cli/rules.hpp>
#include <core/cli/store.hpp>
#include <core/utils/string_utils.hpp>
#include <reversible/circuit.hpp>
#include <reversible/truth_table.hpp>
#include <reversible/cli/stores.hpp>
#include <reversible/functions/circuit_to_truth_table.hpp>
#include <reversible/functions/permutation_to_truth_table.hpp>
#include <reversible/simulation/simple_simulation.hpp>

using namespace boost::program_options;

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

spec_command::spec_command( const environment::ptr& env )
  : command( env, "Specification functions" )
{
  opts.add_options()
    ( "circuit,c",                            "Read from current circuit" )
    ( "permutation,p", value( &permutation ), "Create spec from permutation (starts with 0, space separated)" )
    ( "new,n",                                "Add a new entry to the store; if not set, the current entry is overriden" )
    ;
}

command::rules_t spec_command::validity_rules() const
{
  return {
    { [&]() { return opts.is_set( "circuit" ) != opts.is_set( "permutation" ); }, "either circuit or permutation must be set" },
    { [&]() { return !opts.is_set( "circuit" ) || env->store<circuit>().current_index() >= 0; }, "no circuit in store" }
  };
}

bool spec_command::execute()
{
  auto& specs = env->store<binary_truth_table>();

  if ( specs.empty() || opts.is_set( "new" ) )
  {
    specs.extend();
  }

  if ( opts.is_set( "circuit" ) )
  {
    auto& circuits = env->store<circuit>();

    const auto& circ = circuits.current();

    binary_truth_table spec;
    circuit_to_truth_table( circ, spec, simple_simulation_func() );

    specs.current() = spec;
  }
  else if ( opts.is_set( "permutation" ) )
  {
    std::vector<unsigned> perm;
    parse_string_list( perm, permutation );

    specs.current() =  permutation_to_truth_table( perm );
  }

  return true;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
