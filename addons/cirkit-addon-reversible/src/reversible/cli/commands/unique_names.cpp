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

#include "unique_names.hpp"

#include <alice/rules.hpp>

#include <core/utils/program_options.hpp>
#include <reversible/circuit.hpp>
#include <reversible/cli/stores.hpp>

namespace cirkit
{

unique_names_command::unique_names_command( const environment::ptr& env )
  : cirkit_command( env, "Ensure unique I/O names" )
{
  opts.add_options()
    ( "input_pattern",    value_with_default( &input_pattern ),    "pattern for input names" )
    ( "output_pattern",   value_with_default( &output_pattern ),   "pattern for output names" )
    ( "constant_pattern", value_with_default( &constant_pattern ), "pattern for constant names" )
    ( "garbage_pattern",  value_with_default( &garbage_pattern ),  "pattern for garbage names" )
    ;

}

command::rules_t unique_names_command::validity_rules() const
{
  return {has_store_element<circuit>( env )};
}

bool unique_names_command::execute()
{
  auto& circ = env->store<circuit>().current();

  std::vector<std::string> inputs, outputs;

  auto ictr = 0u, octr = 0u, cctr = 0u, gctr = 0u;
  for ( auto i = 0u; i < circ.lines(); ++i )
  {
    if ( (bool)circ.constants()[i] )
    {
      inputs.push_back( boost::str( boost::format( constant_pattern ) % ++cctr ) );
    }
    else
    {
      inputs.push_back( boost::str( boost::format( input_pattern ) % ++ictr ) );
    }

    if ( circ.garbage()[i] )
    {
      outputs.push_back( boost::str( boost::format( garbage_pattern ) % ++gctr ) );
    }
    else
    {
      outputs.push_back( boost::str( boost::format( output_pattern ) % ++octr ) );
    }
  }

  circ.set_inputs( inputs );
  circ.set_outputs( outputs );

  return true;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
