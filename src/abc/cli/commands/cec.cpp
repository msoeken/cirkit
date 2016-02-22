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

#include "cec.hpp"

#include <core/utils/program_options.hpp>

#include <abc/functions/abc_cec.hpp>

#include <classical/cli/stores.hpp>

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

cec_command::cec_command( const environment::ptr& env )
  : command( env, "Combinatorial equivalence checking of two aigs" ),
    aigs( env->store<aig_graph>() ),
    cex( env->store<counterexample_t>() )
{
  opts.add_options()
    ( "circuit1",  value_with_default( &circ1 ),  "Store-ID of circuit1" )
    ( "circuit2",  value_with_default( &circ2 ),  "Store-ID of circuit2" )
    ( "new,n",                                    "Write counterexample into new store element" )
    ;
  be_verbose();
}

command::rules_t cec_command::validity_rules() const
{
  return {
    {[&]() { return circ1 < aigs.size(); }, "store-ID of circuit1 is invalid" },
    {[&]() { return circ2 < aigs.size(); }, "store-ID of circuit2 is invalid" }
  };
}

bool cec_command::execute()
{
  const auto& aig_circ1 = aigs[circ1];
  const auto& aig_circ2 = aigs[circ2];

  auto settings = make_settings();
  auto statistics = std::make_shared<properties>();
  boost::optional< counterexample_t > cex_result = abc_cec( aig_circ1, aig_circ2, settings, statistics );
  std::cout << boost::format( "[i] cec run-time: %.2f secs" ) % statistics->get<double>( "runtime" ) << std::endl;

  if ( cex_result )
  {
    if ( cex.empty() || opts.is_set( "new" ) )
    {
      cex.extend();
    }
    cex.current() = *cex_result;
    std::cout << "[i] counterexample: " << *cex << std::endl;
  }
  else
  {
    std::cout << "[i] functionally equivalent: no counterexample" << std::endl;
  }
  return true;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
