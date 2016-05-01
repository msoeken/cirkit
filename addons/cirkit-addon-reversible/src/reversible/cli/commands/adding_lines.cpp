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

#include <core/cli/rules.hpp>
#include <core/utils/program_options.hpp>
#include <reversible/cli/stores.hpp>
#include <reversible/optimization/adding_lines.hpp>

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
  : command( env, "Adding lines optimization" )
{
  opts.add_options()
    ( "additional_lines", value_with_default( &additional_lines ), "Number of additional lines" )
    ( "new,n",                                                     "Add new circuit to store" )
    ;
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
  settings->set( "additional_lines", additional_lines );
  adding_lines( opt, circuits.current(), settings, statistics );

  if ( is_set( "new" ) )
  {
    circuits.extend();
  }

  circuits.current() = opt;

  std::cout << boost::format( "[i] run-time: %.2f secs" ) % statistics->get<double>( "runtime" ) << std::endl;

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
