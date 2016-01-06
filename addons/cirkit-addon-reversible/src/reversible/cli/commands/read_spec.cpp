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

#include "read_spec.hpp"

#include <iostream>

#include <core/cli/rules.hpp>
#include <core/utils/program_options.hpp>
#include <reversible/truth_table.hpp>
#include <reversible/cli/stores.hpp>
#include <reversible/io/read_specification.hpp>

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

read_spec_command::read_spec_command( const environment::ptr& env )
  : command( env, "Read specification" )
{
  opts.add_options()
    ( "filename", value( &filename ), "Filename for the *.spec file" )
    ( "new,n",                        "Add a new entry to the store; if not set, the current entry is overriden" )
    ;
}

command::rules_t read_spec_command::validity_rules() const
{
  return {
    file_exists( filename, "filename" )
  };
}

bool read_spec_command::execute()
{
  auto& specs = env->store<binary_truth_table>();

  if ( specs.empty() || opts.is_set( "new" ) )
  {
    specs.extend();
  }

  read_specification( specs.current(), filename );

  return true;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
