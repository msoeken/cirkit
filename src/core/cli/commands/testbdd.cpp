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

#include "testbdd.hpp"

#include <iostream>
#include <vector>

#include <boost/range/algorithm.hpp>

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

testbdd_command::testbdd_command( const environment::ptr& env ) : command( env, "Tests some BDD routines" )
{
  opts.add_options()
    ( "num_vars,n",    value_with_default( &num_vars ), "Number of variables" )
    ( "cardinality,c", value( &cardinality ),           "Create cardinality constraint" )
    ;
  be_verbose();
}

bool testbdd_command::execute()
{
  if ( opts.is_set( "cardinality" ) )
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

  return true;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
