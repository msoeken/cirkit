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

#include "read_bench.hpp"

#include <core/cli/rules.hpp>
#include <core/utils/program_options.hpp>
#include <classical/cli/stores.hpp>
#include <classical/io/read_bench.hpp>

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

read_bench_command::read_bench_command( const environment::ptr& env )
  : command( env, "Reads an AIG from BENCH" ),
    aigs( env->store<aig_graph>() )
{
  opts.add_options()
    ( "filename", value( &filename ), "BENCH filename" )
    ( "new,n",                        "Add a new entry to the store; if not set, the current entry is overriden" )
    ;
  be_verbose();
}

command::rules_t read_bench_command::validity_rules() const
{
  return { file_exists(filename, "filename") };
}

bool read_bench_command::execute()
{
  if ( is_verbose() )
  {
    std::cout << "[i] read from " << filename << std::endl;
  }
  if ( aigs.empty() || is_set( "new" ))
  {
    aigs.extend();
  }
  else
  {
    aigs.current() = aig_graph();
  }

  read_bench( aigs.current(), filename );

  return true;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
