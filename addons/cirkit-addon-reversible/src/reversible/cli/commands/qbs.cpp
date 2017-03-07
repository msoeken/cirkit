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

#include "qbs.hpp"

#include <boost/format.hpp>

#include <alice/rules.hpp>

#include <core/utils/program_options.hpp>
#include <classical/optimization/esop_minimization.hpp>
#include <classical/optimization/exorcism_minimization.hpp>
#include <reversible/circuit.hpp>
#include <reversible/rcbdd.hpp>
#include <reversible/cli/stores.hpp>
#include <reversible/synthesis/qmdd_synthesis.hpp>

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

qbs_command::qbs_command( const environment::ptr& env )
  : cirkit_command( env, "QMDD based synthesis" )
{
  be_verbose();
  opts.add_options()
    ( "esop_minimizer", value_with_default( &esop_minimizer ), "ESOP minizer (0: built-in, 1: exorcism); only with symbolic approach" )
    ( "new,n",                                                 "Creates new entry in store for circuit" )
    ;
}

command::rules_t qbs_command::validity_rules() const
{
  return { has_store_element<rcbdd>( env ) };
}

bool qbs_command::execute()
{
  const auto& rcbdds = env->store<rcbdd>();
  auto& circuits = env->store<circuit>();

  if ( circuits.empty() || is_set( "new" ) )
  {
    circuits.extend();
  }

  const auto settings = make_settings();

  auto esopmin_settings = std::make_shared<properties>();
  esopmin_settings->set( "verbose", is_verbose() );
  settings->set( "esopmin", esop_minimizer ? dd_based_exorcism_minimization_func( esopmin_settings ) : dd_based_esop_minimization_func( esopmin_settings ) );

  qmdd_synthesis( circuits.current(), rcbdds.current(), settings, statistics );

  std::cout << boost::format( "[i] run-time: %.2f secs" ) % statistics->get<double>( "runtime" ) << std::endl;

  return true;
}

command::log_opt_t qbs_command::log() const
{
  return log_opt_t({
      {"runtime", statistics->get<double>( "runtime" )}
    });
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
