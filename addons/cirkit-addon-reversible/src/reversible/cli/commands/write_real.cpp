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

#include "write_real.hpp"

#include <core/cli/rules.hpp>
#include <core/utils/program_options.hpp>
#include <reversible/circuit.hpp>
#include <reversible/cli/stores.hpp>
#include <reversible/io/write_realization.hpp>

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

write_real_command::write_real_command( const environment::ptr& env )
  : command( env, "Write realization" )
{
  opts.add_options()
    ( "filename", value( &filename ), "Filename for the *.real file" )
    ;
}

command::rules_t write_real_command::validity_rules() const
{
  return {
    { [&]() { return opts.is_set( "filename" ); }, "no filename given" },
    has_store_element<circuit>( env )
  };
}

bool write_real_command::execute()
{
  write_realization( env->store<circuit>().current(), filename );
  return true;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
