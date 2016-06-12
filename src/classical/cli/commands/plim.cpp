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

#include "plim.hpp"

#include <boost/format.hpp>

#include <core/utils/program_options.hpp>
#include <core/utils/range_utils.hpp>
#include <classical/plim/plim_compiler.hpp>

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

plim_command::plim_command( const environment::ptr& env )
  : mig_base_command( env, "PLiM compiler" )
{
  opts.add_options()
    ( "print,p",                                                         "print the program" )
    ( "generator_strategy,s", value_with_default( &generator_strategy ), "memristor generator request strategy:\n0: LIFO\n1: FIFO" )
    ( "naive",                                                           "turn off all optimization" )
    ( "progress",                                                        "show progress" )
    ;
  be_verbose();
}

bool plim_command::execute()
{
  const auto settings = make_settings();
  settings->set( "enable_cost_function", !is_set( "naive" ) );
  settings->set( "generator_strategy", generator_strategy );
  settings->set( "progress", is_set( "progress" ) );
  const auto program = compile_for_plim( mig(), settings, statistics );

  if ( is_set( "progress" ) )
  {
    std::cout << std::endl;
  }

  if ( is_set( "print" ) )
  {
    std::cout << program << std::endl;
  }

  std::cout << boost::format( "[i] run-time:     %.2f secs" ) % statistics->get<double>( "runtime" ) << std::endl;
  std::cout << "[i] step count:   " << program.step_count() << std::endl
            << "[i] RRAM count:   " << program.rram_count() << std::endl
            << "[i] write counts: " << any_join( program.write_counts(), " " ) << std::endl;

  return true;
}

command::log_opt_t plim_command::log() const
{
  return log_opt_t({
      {"runtime", statistics->get<double>( "runtime" )},
      {"step_count", statistics->get<int>( "step_count" )},
      {"rram_count", statistics->get<int>( "rram_count" )},
      {"write_counts", statistics->get<std::vector<int>>( "write_counts" )}
    });
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
