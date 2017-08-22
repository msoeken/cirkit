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

#include "mig_rewrite.hpp"

#include <boost/format.hpp>

#include <core/utils/program_options.hpp>
#include <classical/mig/mig_rewriting.hpp>

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

mig_rewrite_command::mig_rewrite_command( const environment::ptr& env ) : mig_base_command( env, "MIG rewriting" )
{
  opts.add_options()
    ( "metric",   value_with_default( &metric ),   "Cost metric for optimization\n0: depth\n1: area\n2: memristor" )
    ( "nodist",                                    "Don't use distributivity rule" )
    ( "noassoc",                                   "Don't use associativity rule" )
    ( "nocassoc",                                  "Don't use complementary associativity rule" )
    ( "strategy", value_with_default( &strategy ), "Stategy for memristor optimized rewriting:\n0: multi-objective\n1: RRAM step\n2: PLiM\n3: only inverters" )
    ( "effort,e", value_with_default( &effort ),   "Number of optimization cycles" )
    ;
  be_verbose();
}

bool mig_rewrite_command::execute()
{
  const auto settings = make_settings();
  settings->set( "effort", effort );
  settings->set( "use_distributivity", !is_set( "nodist" ) );
  settings->set( "use_associativity", !is_set( "noassoc" ) );
  settings->set( "use_compl_associativity", !is_set( "nocassoc" ) );
  settings->set( "strategy", strategy );

  switch ( metric )
  {
  case 0u:
    mig() = mig_depth_rewriting( mig(), settings, statistics );
    break;
  case 1u:
    mig() = mig_area_rewriting( mig(), settings, statistics );
    break;
  case 2u:
    mig() = mig_memristor_rewriting( mig(), settings, statistics );
    break;
  }

  std::cout << boost::format( "[i] run-time: %.2f secs" ) % statistics->get<double>( "runtime" ) << std::endl;

  if ( is_verbose() )
  {
    std::cout << boost::format( "[i] distributivity: %d" ) % statistics->get<unsigned>( "distributivity_count" ) << std::endl
              << boost::format( "[i] associativity: %d" ) % statistics->get<unsigned>( "associativity_count" ) << std::endl
              << boost::format( "[i] complementary associativity: %d" ) % statistics->get<unsigned>( "compl_associativity_count" ) << std::endl;
  }

  return true;
}

command::log_opt_t mig_rewrite_command::log() const
{
  return log_opt_t({
      {"metric", static_cast<int>( metric )},
      {"strategy", static_cast<int>( strategy )},
      {"effort", static_cast<int>( effort )},
      {"runtime", statistics->get<double>( "runtime" )},
      {"distributivity_count", static_cast<int>( statistics->get<unsigned>( "distributivity_count" ) )},
      {"associativity_count", static_cast<int>( statistics->get<unsigned>( "associativity_count" ) )},
      {"compl_associativity_count", static_cast<int>( statistics->get<unsigned>( "compl_associativity_count" ) )}
    });
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
