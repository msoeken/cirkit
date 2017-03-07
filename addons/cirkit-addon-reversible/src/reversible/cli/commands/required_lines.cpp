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

#include "required_lines.hpp"

#include <alice/rules.hpp>

#include <core/cli/stores.hpp>
#include <reversible/functions/calculate_additional_lines.hpp>

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

required_lines_command::required_lines_command( const environment::ptr& env )
  : cirkit_command( env, "Calculates number of required lines" )
{
  be_verbose();
}

command::rules_t required_lines_command::validity_rules() const
{
  return {has_store_element<bdd_function_t>( env )};
}

bool required_lines_command::execute()
{
  const auto& bdds = env->store<bdd_function_t>();

  const auto settings = make_settings();

  additional = calculate_additional_lines( bdds.current(), settings, statistics );

  std::cout << "[i] inputs:     " << statistics->get<unsigned>( "num_inputs" ) << std::endl
            << "[i] outputs:    " << statistics->get<unsigned>( "num_outputs" ) << std::endl
            << "[i] additional: " << additional << std::endl
            << boost::format( "[i] run-time:   %.2f secs" ) % statistics->get<double>( "runtime" ) << std::endl;

  return true;
}

command::log_opt_t required_lines_command::log() const
{
  return log_opt_t({
      {"additional", additional},
      {"num_inputs", statistics->get<unsigned>( "num_inputs" )},
      {"num_outputs", statistics->get<unsigned>( "num_outputs" )},
      {"runtime", statistics->get<double>( "runtime" )}
    });
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
