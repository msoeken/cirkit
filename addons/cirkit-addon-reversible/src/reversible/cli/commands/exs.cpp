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

#include "exs.hpp"

#include <boost/format.hpp>

#include <core/cli/rules.hpp>
#include <reversible/circuit.hpp>
#include <reversible/truth_table.hpp>
#include <reversible/cli/stores.hpp>
#include <reversible/synthesis/exact_synthesis.hpp>
#include <reversible/synthesis/quantified_exact_synthesis.hpp>

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

exs_command::exs_command( const environment::ptr& env )
  : command( env, "Exact synthesis" )
{
  opts.add_options()
    ( "mode",                value_with_default( &mode ),      "Mode (0: BDD, 1: SAT)" )
    ( "max_depth",           value_with_default( &max_depth ), "Maximum search depth" )
    ( "negative,n",                                            "Allow negative control lines" )
    ( "multiple,m",                                            "Allow multiple target lines (only with SAT)" )
    ( "all_solutions,a",                                       "Extract all solutions (only with BDD)" )
    ( "new",                                                   "Creates a new circuit" )
    ;
  be_verbose();
}

command::rules_t exs_command::validity_rules() const
{
  return {
    { [this]() { return this->mode <= 1u; }, "mode must be either 0 or 1" },
    has_store_element<binary_truth_table>( env )
  };
}

bool exs_command::execute()
{
  auto& circuits = env->store<circuit>();
  auto& specs    = env->store<binary_truth_table>();

  if ( circuits.empty() || opts.is_set( "new" ) )
  {
    circuits.extend();
  }

  auto settings = make_settings();
  settings->set( "max_depth",     max_depth );
  settings->set( "negative",      opts.is_set( "negative" ) );
  settings->set( "multiple",      opts.is_set( "multiple" ) );
  settings->set( "all_solutions", opts.is_set( "all_solutions" ) );

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

  circuits.current() = circ;

  if ( mode == 0u && result )
  {
    //std::cout << "[i] number of solutions: " << statistics->get<unsigned>( "num_circuits" ) << std::endl;
  }

  std::cout << boost::format( "[i] run-time: %.2f secs" ) % statistics->get<double>( "runtime" ) << std::endl;

  if ( mode == 0u && result && opts.is_set( "all_solutions" ) )
  {
    auto current_index = circuits.current_index();

    for ( const auto& sol : statistics->get<std::vector<circuit>>( "solutions" ) )
    {
      circuits.extend();
      circuits.current() = sol;
    }

    circuits.set_current_index( current_index );
  }

  return true;
}

command::log_opt_t exs_command::log() const
{
  return log_opt_t({
      { "runtime", statistics->get<double>( "runtime" ) }//,
      //{ "num_circuits", static_cast<int>( statistics->get<unsigned>( "num_circuits" ) ) }
    });
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
