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
  : cirkit_command( env, "Combinatorial equivalence checking of two aigs" )
{
  opts.add_options()
    ( "circuit1",  value_with_default( &circ1 ),  "store-ID of circuit1" )
    ( "circuit2",  value_with_default( &circ2 ),  "store-ID of circuit2" )
    ;

  if ( env->has_store<counterexample_t>() )
  {
    add_new_option();
  }
  be_verbose();
}

command::rules_t cec_command::validity_rules() const
{
  return {
    {[&]() { return circ1 < env->store<aig_graph>().size(); }, "store-ID of circuit1 is invalid" },
    {[&]() { return circ2 < env->store<aig_graph>().size(); }, "store-ID of circuit2 is invalid" }
  };
}

bool cec_command::execute()
{
  auto& aigs = env->store<aig_graph>();

  const auto& aig_circ1 = aigs[circ1];
  const auto& aig_circ2 = aigs[circ2];

  auto settings = make_settings();
  boost::optional<counterexample_t> cex_result = abc_cec( aig_circ1, aig_circ2, settings, statistics );
  print_runtime();

  if ( (bool)cex_result )
  {
    if ( env->has_store<counterexample_t>() )
    {
      auto& cex = env->store<counterexample_t>();
      extend_if_new( cex );
      cex.current() = *cex_result;
    }
    std::cout << "[i] counterexample: " << *cex_result << std::endl;
  }
  else
  {
    std::cout << "[i] functionally equivalent: no counterexample" << std::endl;
  }
  return true;
}

command::log_opt_t cec_command::log() const
{
  return log_opt_t({{"runtime", statistics->get<double>( "runtime" )}});
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
