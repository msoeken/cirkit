/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2016  EPFL
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

#include "bdd.hpp"

#include <vector>

#include <alice/rules.hpp>

#include <core/cli/stores.hpp>

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

bdd_command::bdd_command( const environment::ptr& env )
  : cirkit_command( env, "BDD manipulation" )
{
  opts.add_options()
    ( "characteristic,c", value( &characteristic ), "Compute characteristic function (x: inputs first, y: outputs first)" )
    ( "new,n",                                      "Add a new entry to the store; if not set, the current entry is overriden" )
    ;
}

command::rules_t bdd_command::validity_rules() const
{
  return { has_store_element<bdd_function_t>( env ) };
}

bool bdd_command::execute()
{
  auto& bdds = env->store<bdd_function_t>();

  if ( is_set( "characteristic" ) )
  {
    auto bdd = bdds.current();

    if ( is_set( "new" ) )
    {
      bdds.extend();
    }

    bdds.current() = compute_characteristic( bdd, characteristic == "x" );
  }

  return true;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
