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

#include "required_lines.hpp"

#include <alice/rules.hpp>

#include <core/cli/stores.hpp>
#include <reversible/functions/calculate_additional_lines.hpp>

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

required_lines_command::required_lines_command( const environment::ptr& env )
  : cirkit_command( env, "Calculates number of required lines" )
{
  be_verbose();
}

command::rules_t required_lines_command::validity_rules() const
{
  return {has_store_element<bdd_function_t>( env )};
}

bool required_lines_command::execute()
{
  const auto& bdds = env->store<bdd_function_t>();

  const auto settings = make_settings();

  additional = calculate_additional_lines( bdds.current(), settings, statistics );

  std::cout << "[i] inputs:     " << statistics->get<unsigned>( "num_inputs" ) << std::endl
            << "[i] outputs:    " << statistics->get<unsigned>( "num_outputs" ) << std::endl
            << "[i] additional: " << additional << std::endl
            << boost::format( "[i] run-time:   %.2f secs" ) % statistics->get<double>( "runtime" ) << std::endl;

  return true;
}

command::log_opt_t required_lines_command::log() const
{
  return log_opt_t({
      {"additional", additional},
      {"num_inputs", statistics->get<unsigned>( "num_inputs" )},
      {"num_outputs", statistics->get<unsigned>( "num_outputs" )},
      {"runtime", statistics->get<double>( "runtime" )}
    });
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
