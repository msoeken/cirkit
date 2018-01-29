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

#ifdef ADDON_FORMAL

#include "exs.hpp"

#include <alice/rules.hpp>

#include <reversible/circuit.hpp>
#include <reversible/truth_table.hpp>
#include <cli/reversible_stores.hpp>
#include <reversible/synthesis/exact_synthesis.hpp>
#include <reversible/synthesis/exact_toffoli_synthesis.hpp>
#include <reversible/synthesis/quantified_exact_synthesis.hpp>

namespace cirkit
{

exs_command::exs_command( const environment::ptr& env )
  : cirkit_command( env, "Exact synthesis" )
{
  add_option( "--mode,-m", mode, "mode (0: BDD, 1: SAT, 2: SAT (Toffoli))", true );
  add_option( "--start_depth,-s", start_depth, "initial search depth", true );
  add_option( "--max_depth", max_depth, "maximum search depth", true );
  add_flag( "--negative,-n", "allow negative control lines" );
  add_flag( "--multiple", "allow multiple target lines (only with SAT)" );
  add_flag( "--all_solutions,-a", "extract all solutions (only with BDD)" );
  add_new_option( false );
  be_verbose();
}

command::rules exs_command::validity_rules() const
{
  return {
    { [this]() { return this->mode <= 2u; }, "mode must be either 0 or 1" },
    has_store_element<binary_truth_table>( env )
  };
}

void exs_command::execute()
{
  auto& circuits = env->store<circuit>();
  auto& specs    = env->store<binary_truth_table>();

  extend_if_new( circuits );

  auto settings = make_settings();
  settings->set( "start_depth",   start_depth );
  settings->set( "max_depth",     max_depth );
  settings->set( "negative",      is_set( "negative" ) );
  settings->set( "multiple",      is_set( "multiple" ) );
  settings->set( "all_solutions", is_set( "all_solutions" ) );

  circuit circ;
  auto result = false;

  if ( mode == 0u )
  {
    result = quantified_exact_synthesis( circ, specs.current(), settings, statistics );
  }
  else if ( mode == 1u )
  {
    result = exact_synthesis( circ, specs.current(), settings, statistics );
  }
  else if ( mode == 2u )
  {
    result = exact_toffoli_synthesis( circ, specs.current(), settings, statistics );
  }

  circuits.current() = circ;

  if ( mode == 0u && result )
  {
    //std::cout << "[i] number of solutions: " << statistics->get<unsigned>( "num_circuits" ) << std::endl;
  }

  print_runtime();

  if ( mode == 0u && result && is_set( "all_solutions" ) )
  {
    auto current_index = circuits.current_index();

    for ( const auto& sol : statistics->get<std::vector<circuit>>( "solutions" ) )
    {
      circuits.extend();
      circuits.current() = sol;
    }

    circuits.set_current_index( current_index );
  }
}

nlohmann::json exs_command::log() const
{
  return nlohmann::json({
      { "runtime", statistics->get<double>( "runtime" ) }//,
      //{ "num_circuits", static_cast<int>( statistics->get<unsigned>( "num_circuits" ) ) }
    });
}

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
