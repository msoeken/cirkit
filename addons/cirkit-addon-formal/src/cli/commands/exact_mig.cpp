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

#include "exact_mig.hpp"

#include <boost/optional.hpp>

#include <cli/stores.hpp>
#include <classical/mig/mig.hpp>
#include <classical/mig/mig_from_string.hpp>
#include <classical/mig/mig_utils.hpp>
#include <classical/utils/truth_table_utils.hpp>
#include <formal/synthesis/exact_mig.hpp>

#include <fmt/format.h>

namespace cirkit
{

exact_mig_command::exact_mig_command( const environment::ptr& env )
  : cirkit_command( env, "Exact MIG synthesis" )
{
  add_option( "--objective,-o", objective, "optimization objective:\n0: size-optimum\n1: size/depth-optimum\n2: depth/size-optimum", true );
  add_option( "--start,-s", start, "start value for gate enumeration", true );
  add_option( "--stop", stop, "stop value for gate enumeration (ignored for depth/size-optimum)" );
  add_option( "--start_depth", start_depth, "start value for depth enumeration", true );
  add_flag( "--mig,-m", "load spec from MIG instead of truth table" );
  add_flag( "--incremental,-i", "incremental SAT solving" );
  add_option( "--max_solutions", max_solutions, "enumerate as many solutions (only with non incremental)", true );
  add_option( "--breaking", breaking, "symmetry breaking\ns: structural hashing\na: associativity\nl: co-lexicographic ordering\nt: support\ny: symmetric variables", true );
  add_flag( "--print_solutions", "print solutions" );
  add_flag( "--enc_int", "encode numbers as integers (not bit-vectors)" );
  add_option( "--timeout", timeout, "timeout (in seconds)" );
  add_flag( "--timeout_heuristic", "continue with next level on timeout" );
  add_flag( "--very_verbose", "be very verbose" );
  be_verbose();
}

void exact_mig_command::execute()
{
  auto settings = make_settings();
  settings->set( "objective",           objective );
  settings->set( "start",               start );
  if ( is_set( "stop" ) )
  {
    settings->set( "stop",              stop );
  }
  settings->set( "start_depth",         start_depth );
  settings->set( "incremental",         is_set( "incremental" ) );
  settings->set( "max_solutions",       max_solutions );
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

  if ( is_set( "mig" ) )
  {
    auto new_mig = exact_mig_with_sat( migs.current(), settings, statistics );
    if ( (bool)new_mig )
    {
      migs.extend();
      migs.current() = *new_mig;
    }
    else
    {
      std::cout << "[w] timeout" << std::endl;
    }
  }
  else
  {
    auto new_mig = exact_mig_with_sat( tts.current(), settings, statistics );
    if ( (bool)new_mig )
    {
      migs.extend();
      migs.current() = *new_mig;
    }
    else
    {
      std::cout << "[w] timeout" << std::endl;
    }
  }

  if ( is_set( "print_solutions" ) )
  {
    const auto& solutions = statistics->get<std::vector<mig_graph>>( "all_solutions" );

    std::cout << fmt::format( "[i] found {} solutions", solutions.size() ) << std::endl;

    for ( const auto& smig : solutions )
    {
      auto expr = mig_to_expression( smig, mig_info( smig ).outputs.front().first );
      std::cout << expr << std::endl;
    }
  }

  print_runtime();
  std::cout << fmt::format( "[i] memory: {:.2f} MB", statistics->get<double>( "memory" ) ) << std::endl;
}

nlohmann::json exact_mig_command::log() const
{
  return nlohmann::json({
      {"runtime",       statistics->get<double>( "runtime" )},
      {"start",         start},
      {"all_solutions", is_set( "all_solutions" )},
      {"min_depth",     is_set( "min_depth" )}
    });
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
