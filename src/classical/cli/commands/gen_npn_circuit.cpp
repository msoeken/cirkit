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

#include "gen_npn_circuit.hpp"

#include <iostream>

#include <core/utils/program_options.hpp>
#include <classical/generators/npn_circuit.hpp>

namespace cirkit
{

gen_npn_circuit_command::gen_npn_circuit_command( const environment::ptr& env )
  : cirkit_command( env, "Generates NPN classification circuits" )
{
  opts.add_options()
    ( "filename",   value_with_default( &filename ), "Verilog filename for the result" )
    ( "num_vars,n", value_with_default( &num_vars ), "number of variables (from 2 to 6) " )
    ;
  add_positional_option( "num_vars" );
}

command::rules_t gen_npn_circuit_command::validity_rules() const
{
  return {{[this]() { return num_vars >= 2 && num_vars <= 6; }, "number of variables must be between 2 and 6"}};
}

bool gen_npn_circuit_command::execute()
{
  std::ofstream os( filename.c_str(), std::ofstream::out );
  generate_npn_circuit( os, num_vars );

  return true;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
