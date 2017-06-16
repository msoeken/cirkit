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

#include "concat.hpp"

#include <core/utils/program_options.hpp>
#include <reversible/cli/stores.hpp>
#include <reversible/functions/add_circuit.hpp>
#include <reversible/functions/copy_circuit.hpp>

namespace cirkit
{

concat_command::concat_command( const environment::ptr& env )
  : cirkit_command( env, "Concats two circuits" )
{
  opts.add_options()
    ( "id1", value_with_default( &id1 ), "index of first circuit (determines number of lines)" )
    ( "id2", value_with_default( &id2 ), "index of second circuit" )
    ;

  add_new_option();
}

command::rules_t concat_command::validity_rules() const
{
  return {
    {[this]() { return id1 < env->store<circuit>().size(); }, "first index is out of range"},
    {[this]() { return id2 < env->store<circuit>().size(); }, "second index is out of range"}
  };
}

bool concat_command::execute()
{
  auto& circuits = env->store<circuit>();

  circuit circ;
  copy_circuit( circuits[id1], circ );
  append_circuit( circ, circuits[id2] );

  extend_if_new( circuits );

  circuits.current() = circ;

  return true;
}

command::log_opt_t concat_command::log() const
{
  return log_map_t({
      {"id1", id1},
      {"id2", id2}
    });
}


}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
