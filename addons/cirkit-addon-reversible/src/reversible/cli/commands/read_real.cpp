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

#include "read_real.hpp"

#include <iostream>

#include <core/cli/rules.hpp>
#include <core/utils/program_options.hpp>
#include <reversible/circuit.hpp>
#include <reversible/cli/stores.hpp>
#include <reversible/functions/circuit_from_string.hpp>
#include <reversible/functions/clear_circuit.hpp>
#include <reversible/io/read_realization.hpp>

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

read_real_command::read_real_command( const environment::ptr& env )
  : command( env, "Read realization" )
{
  add_positional_option( "filename" );
  opts.add_options()
    ( "filename", value( &filename ), "Filename for the *.real file" )
    ( "string,s", value( &string ),   "Read from string (e.g. t3 a b c, t2 a b, t1 a, f3 a b c)" )
    ( "new,n",                        "Add a new entry to the store; if not set, the current entry is overriden" )
    ;
}

command::rules_t read_real_command::validity_rules() const
{
  return {
    { [this]() { return is_set( "filename" ) != is_set( "string" ); }, "either filename or string must be set" },
    file_exists_if_set( *this, filename, "filename" )
  };
}

bool read_real_command::execute()
{
  auto& circuits = env->store<circuit>();

  if ( circuits.empty() || is_set( "new" ) )
  {
    circuits.extend();
  }

  clear_circuit( circuits.current() );
  if ( is_set( "filename" ) )
  {
    read_realization( circuits.current(), filename );
  }
  else
  {
    circuits.current() = circuit_from_string( string );
  }

  return true;
}

command::log_opt_t read_real_command::log() const
{
  return log_opt_t({{"filename", filename}});
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
