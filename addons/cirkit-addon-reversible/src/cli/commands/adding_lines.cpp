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

#include "adding_lines.hpp"

#include <alice/rules.hpp>

#include <cli/reversible_stores.hpp>
#include <reversible/optimization/adding_lines.hpp>
#include <reversible/utils/costs.hpp>

namespace cirkit
{

adding_lines_command::adding_lines_command( const environment::ptr& env )
  : cirkit_command( env, "Adding lines optimization" )
{
  add_option( "--additional_lines,-a", additional_lines, "number of additional lines", true );
  add_option( "--cost_function,-c", costs, "cost function:\n0: Clifford+T costs\n1: T-depth\n2: T-count\n3: H-count\n4: Transistor costs\n5: NCV costs", true );
  add_new_option();
}

command::rules adding_lines_command::validity_rules() const
{
  return {has_store_element<circuit>( env )};
}

void adding_lines_command::execute()
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
}

nlohmann::json adding_lines_command::log() const
{
  return nlohmann::json({
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
