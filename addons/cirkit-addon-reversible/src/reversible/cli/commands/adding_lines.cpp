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

#include "adding_lines.hpp"

#include <boost/format.hpp>

#include <alice/rules.hpp>

#include <core/utils/program_options.hpp>
#include <reversible/cli/stores.hpp>
#include <reversible/optimization/adding_lines.hpp>
#include <reversible/utils/costs.hpp>

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

adding_lines_command::adding_lines_command( const environment::ptr& env )
  : cirkit_command( env, "Adding lines optimization" )
{
  opts.add_options()
    ( "additional_lines,a", value_with_default( &additional_lines ), "number of additional lines" )
    ( "cost_function,c",    value_with_default( &costs ),            "cost function:\n0: Clifford+T costs\n1: T-depth\n2: T-count\n3: H-count\n4: Transistor costs\n5: NCV costs" )
    ;
  add_new_option();
}

command::rules_t adding_lines_command::validity_rules() const
{
  return {has_store_element<circuit>( env )};
}

bool adding_lines_command::execute()
{
  auto& circuits = env->store<circuit>();

  circuit opt;
  auto settings = make_settings();

  std::vector<costs_by_gate_func> cfs = {costs_by_gate_func( clifford_t_quantum_costs() ),
                                         costs_by_gate_func( t_depth_costs() ),
                                         costs_by_gate_func( t_costs() ),
                                         costs_by_gate_func( h_costs() ),
                                         costs_by_gate_func( transistor_costs() ),
                                         costs_by_gate_func( ncv_quantum_costs() )};
  settings->set( "additional_lines", additional_lines );
  settings->set( "cost_function", cost_function( cfs[costs] ) );
  adding_lines( opt, circuits.current(), settings, statistics );

  extend_if_new( circuits );

  circuits.current() = opt;

  print_runtime();

  return true;
}

command::log_opt_t adding_lines_command::log() const
{
  return log_opt_t({
      {"runtime", statistics->get<double>( "runtime" )},
      {"additional_lines", additional_lines}
    });
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
