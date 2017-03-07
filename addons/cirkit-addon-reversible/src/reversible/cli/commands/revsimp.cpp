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

#include "revsimp.hpp"

#include <alice/rules.hpp>
#include <core/utils/program_options.hpp>
#include <reversible/circuit.hpp>
#include <reversible/cli/stores.hpp>
#include <reversible/optimization/simplify.hpp>

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

revsimp_command::revsimp_command( const environment::ptr& env )
  : cirkit_command( env, "Reversible circuit simplification" )
{
  opts.add_options()
    ( "methods",   value_with_default( &methods ), "optimization methods:\nm: try to merge gates with same target\nn: cancel NOT gates\na: merge adjacent gates\ne: resynthesize same-target gates with exorcism\ns: propagate SWAP gates (may change output order)" )
    ( "noreverse",                                 "do not optimize in reverse direction" )
    ;
  be_verbose();
  add_new_option();
}

command::rules_t revsimp_command::validity_rules() const
{
  return {has_store_element<circuit>( env )};
}

bool revsimp_command::execute()
{
  auto& circuits = env->store<circuit>();

  auto settings = make_settings();
  settings->set( "methods",     methods );
  settings->set( "reverse_opt", !is_set( "noreverse" ) );
  circuit circ;
  simplify( circ, circuits.current(), settings, statistics );

  extend_if_new( circuits );
  circuits.current() = circ;

  print_runtime();

  return true;
}

command::log_opt_t revsimp_command::log() const
{
  return log_opt_t({{"runtime", statistics->get<double>( "runtime" )}});
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
