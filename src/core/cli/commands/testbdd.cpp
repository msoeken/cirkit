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

#include "testbdd.hpp"

#include <iostream>
#include <vector>

#include <boost/range/algorithm.hpp>

#include <core/cli/stores.hpp>
#include <core/utils/bdd_utils.hpp>
#include <core/utils/program_options.hpp>
#include <core/utils/timer.hpp>

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

testbdd_command::testbdd_command( const environment::ptr& env ) : cirkit_command( env, "Tests some BDD routines" )
{
  opts.add_options()
    ( "num_vars,n",    value_with_default( &num_vars ), "Number of variables" )
    ( "cardinality,c", value( &cardinality ),           "Create cardinality constraint" )
    ( "up",                                             "Performs up operation" )
    ;
  be_verbose();
}

bool testbdd_command::execute()
{
  if ( is_set( "cardinality" ) )
  {
    print_timer t( "[i] run-time: %w secs\n", true );

    Cudd manager;

    std::vector<BDD> vars( num_vars );
    boost::generate( vars, [&]() { return manager.bddVar(); } );

    const auto f = make_eq( manager, vars, cardinality );

    if ( is_verbose() )
    {
      f.PrintMinterm();
    }
  }

  if ( is_set( "up" ) )
  {
    auto& bdds = env->store<bdd_function_t>();

    auto bdd = bdds.current();
    auto f = bdd_up( bdd.first, bdd.second.front() );
    std::cout << "m: " << is_monotone( bdd.first, bdd.second.front() ) << " " << is_monotone( bdd.first, f ) << std::endl;
    bdds.current().second.front() = f;
  }

  return true;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
