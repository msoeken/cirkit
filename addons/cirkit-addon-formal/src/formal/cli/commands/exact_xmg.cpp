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

#include "exact_xmg.hpp"

#include <boost/format.hpp>
#include <boost/optional.hpp>

#include <core/utils/program_options.hpp>
#include <classical/mig/mig.hpp>
#include <classical/cli/stores.hpp>
#include <classical/utils/truth_table_utils.hpp>
#include <classical/xmg/xmg.hpp>
#include <classical/xmg/xmg_expr.hpp>
#include <formal/synthesis/exact_mig.hpp>

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

exact_xmg_command::exact_xmg_command( const environment::ptr& env )
  : cirkit_command( env, "Exact XMG synthesis" )
{
  opts.add_options()
    ( "objective,o",       value_with_default( &objective ), "optimization objective:\n0: size-optimum\n1: size/depth-optimum\n2: depth/size-optimum" )
    ( "start,s",           value_with_default( &start ),     "start value for gate enumeration" )
    ( "mig,m",                                               "load spec from MIG instead of truth table" )
    ( "incremental,i",                                       "incremental SAT solving" )
    ( "all_solutions,a",                                     "enumerate all solutions (only with non incremental)" )
    ( "breaking",          value_with_default( &breaking ),  "symmetry breaking\ns: structural hashing\na: associativity\nl: co-lexicographic ordering\nt: support" )
    ( "print_solutions",                                     "print solutions" )
    ( "enc_int",                                             "encode numbers as integers (not bit-vectors)" )
    ( "timeout",           value( &timeout ),                "timeout (in seconds)" )
    ( "timeout_heuristic",                                   "continue with next level on timeout" )
    ( "very_verbose",                                        "be very verbose" )
    ;
  be_verbose();
}

bool exact_xmg_command::execute()
{
  using boost::format;

  auto settings = make_settings();
  settings->set( "objective",           objective );
  settings->set( "start",               start );
  settings->set( "incremental",         is_set( "incremental" ) );
  settings->set( "all_solutions",       is_set( "all_solutions" ) );
  settings->set( "breaking",            breaking );
  settings->set( "enc_with_bitvectors", !is_set( "enc_int" ) );
  settings->set( "very_verbose",        is_set( "very_verbose" ) );
  if ( is_set( "timeout" ) )
  {
    settings->set( "timeout", boost::optional<unsigned>( timeout ) );
    settings->set( "timeout_heuristic", is_set( "timeout_heuristic" ) );
  }

  const auto& tts = env->store<tt>();
  auto& migs = env->store<mig_graph>();
  auto& xmgs = env->store<xmg_graph>();

  if ( is_set( "mig" ) )
  {
    auto new_xmg = exact_xmg_with_sat( migs.current(), settings, statistics );
    if ( (bool)new_xmg )
    {
      xmgs.extend();
      xmgs.current() = *new_xmg;
    }
    else
    {
      std::cout << "[w] timeout" << std::endl;
    }
  }
  else
  {
    auto new_xmg = exact_xmg_with_sat( tts.current(), settings, statistics );
    if ( (bool)new_xmg )
    {
      xmgs.extend();
      xmgs.current() = *new_xmg;
    }
    else
    {
      std::cout << "[w] timeout" << std::endl;
    }
  }

  if ( is_set( "all_solutions" ) )
  {
    const auto& solutions = statistics->get<std::vector<xmg_graph>>( "all_solutions" );
    std::cout << format( "[i] found %d solutions" ) % solutions.size() << std::endl;
    if ( is_set( "print_solutions" ) )
    {
      for ( const auto& sxmg : solutions )
      {
        auto expr = xmg_to_expression( sxmg, sxmg.outputs().front().first );
        std::cout << expr << "   " << expression_to_shape( expr, false ) << std::endl;
      }
    }
  }

  std::cout << format( "[i] run-time: %.2f seconds" ) % statistics->get<double>( "runtime" ) << std::endl;

  return true;
}

command::log_opt_t exact_xmg_command::log() const
{
  return log_opt_t({
      {"runtime",       statistics->get<double>( "runtime" )},
      {"last_size",     statistics->get<unsigned>( "last_size" )},
      {"start",         start},
      {"all_solutions", is_set( "all_solutions" )},
      {"objective",     objective},
      {"timeout",       timeout}
    });
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
