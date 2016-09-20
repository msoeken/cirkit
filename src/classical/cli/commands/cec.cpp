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

#include "cec.hpp"

#include <core/utils/program_options.hpp>

#include <classical/abc/functions/abc_cec.hpp>

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
