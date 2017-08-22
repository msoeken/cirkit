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
