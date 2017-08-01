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

#include "stg_as.hpp"

#include <core/utils/program_options.hpp>
#include <classical/cli/stores.hpp>
#include <reversible/cli/stores.hpp>
#include <reversible/functions/add_stg.hpp>

namespace cirkit
{

stg_as_command::stg_as_command( const environment::ptr& env )
  : cirkit_command( env, "Realize single-target from truth table" )
{
  opts.add_options()
    ( "func,f", value_with_default( &func ), "id of truth table to realize" )
    ( "as,a",   value_with_default( &real ), "id of truth table to use in gate" )
    ;
}

command::rules_t stg_as_command::validity_rules() const
{
  return {
    {[this]() { return func < env->store<tt>().size(); }, "func id is invalid"},
    {[this]() { return real < env->store<tt>().size(); }, "as id is invalid"}
  };
}

bool stg_as_command::execute()
{
  const auto& tts = env->store<tt>();
  auto& circuits = env->store<circuit>();

  circuits.extend();

  circuit circ( tt_num_vars( tts[func] ) + 1u );
  add_stg_as_other( circ, tts[func], tts[real] );
  circuits.current() = circ;

  return true;
}

command::log_opt_t stg_as_command::log() const
{
  return boost::none;
}


}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
