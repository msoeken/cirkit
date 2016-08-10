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

#include "esopbs.hpp"

#include <boost/format.hpp>
#include <boost/program_options.hpp>

#include <alice/rules.hpp>

#include <reversible/circuit.hpp>
#include <reversible/cli/stores.hpp>
#include <reversible/synthesis/esop_synthesis.hpp>

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

esopbs_command::esopbs_command( const environment::ptr& env )
  : cirkit_command( env, "ESOP based synthesis" )
{
  add_positional_option( "filename" );
  opts.add_options()
    ( "filename", value( &filename ), "filename to the ESOP file" )
    ( "mct",                          "no negative controls" )
    ( "no_shared_target",             "no shared target" )
    ;
  add_new_option();
  be_verbose();
}

command::rules_t esopbs_command::validity_rules() const
{
  return {file_exists( filename, "filename" )};
}

bool esopbs_command::execute()
{
  auto& circuits = env->store<circuit>();

  extend_if_new( circuits );

  auto settings = make_settings();
  settings->set( "negative_control_lines", !is_set( "mct" ) );
  settings->set( "share_cube_on_target", !is_set( "no_shared_target" ) );
  esop_synthesis( circuits.current(), filename, settings, statistics );

  print_runtime();

  return true;
}

command::log_opt_t esopbs_command::log() const
{
  return log_opt_t({
      {"mct", is_set( "mct" )},
      {"no_shared_target", is_set( "no_shared_target" )},
      {"runtime", statistics->get<double>( "runtime" )}
    });
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
