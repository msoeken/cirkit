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

#include "rec.hpp"

#include <iostream>

#include <core/utils/program_options.hpp>
#include <reversible/cli/stores.hpp>
#include <reversible/verification/xorsat_equivalence_check.hpp>

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

rec_command::rec_command( const environment::ptr& env )
  : cirkit_command( env, "Equivalence checking for reversible circuits", "[L.G. Amaru, P.-E. Gaillardon, R. Wille, and G. De Micheli, DATE 2016]" )
{
  opts.add_options()
    ( "id1",            value_with_default( &id1 ), "ID of first circuit" )
    ( "id2",            value_with_default( &id2 ), "ID of second circuit" )
    ( "name_mapping,n",                             "map circuits by name instead by index" )
    ;
  be_verbose();
}

bool rec_command::execute()
{
  const auto& circuits = env->store<circuit>();

  auto settings = make_settings();
  settings->set( "name_mapping", is_set( "name_mapping" ) );
  result = xorsat_equivalence_check( circuits[id1], circuits[id2], settings, statistics );

  print_runtime();

  if ( result )
  {
    std::cout << "[i] circuits are \033[1;32mequivalent\033[0m" << std::endl;
  }
  else
  {
    std::cout << "[i] circuits are \033[1;31mnot equivalent\033[0m" << std::endl;
  }

  return true;
}

command::log_opt_t rec_command::log() const
{
  return log_opt_t({
      {"runtime", statistics->get<double>( "runtime" )},
      {"equivalent", result}
    });
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
