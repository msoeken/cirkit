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

#include "random_circuit.hpp"

#include <chrono>
#include <random>

#include <boost/program_options.hpp>

#include <alice/rules.hpp>

#include <core/utils/bitset_utils.hpp>
#include <core/utils/program_options.hpp>
#include <reversible/circuit.hpp>
#include <reversible/gate.hpp>
#include <reversible/target_tags.hpp>
#include <reversible/cli/stores.hpp>

using boost::program_options::value;

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

void create_random_gate( gate& g, unsigned lines, bool negative, std::default_random_engine& generator )
{
  std::uniform_int_distribution<unsigned> dist( 0u, lines - 1u );
  std::uniform_int_distribution<unsigned> bdist( 0u, 1u );

  auto controls = random_bitset( lines, generator );
  auto target   = dist( generator );

  g.set_type( toffoli_tag() );
  g.add_target( target );
  auto pos = controls.find_first();
  while ( pos != controls.npos )
  {
    if ( pos != target )
    {
      g.add_control( make_var( pos, negative ? ( bdist( generator ) == 1u ) : true ) );
    }
    pos = controls.find_next( pos );
  }
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

random_circuit_command::random_circuit_command( const environment::ptr& env )
  : cirkit_command( env, "Creates a random reversible circuit" )
{
  opts.add_options()
    ( "lines",         value_with_default( &lines ), "Number of lines" )
    ( "gates",         value_with_default( &gates ), "Number of gates" )
    ( "negative,n",                                  "Allow negative control lines" )
    ( "insert_gate,g",                               "Doesn't create a circuit, but inserts random gate into current circuit" )
    ( "seed",          value( &seed ),               "Random seed (if not given, current time is used)" )
    ( "new",                                         "Create a new store element" )
    ;
}

command::rules_t random_circuit_command::validity_rules() const
{
  return {
    {[this]() { return !is_set( "insert_gate" ) || env->store<circuit>().current_index() >= 0; }, "no circuit available"}
  };
}

bool random_circuit_command::execute()
{
  if ( !is_set( "seed" ) )
  {
    seed = std::chrono::system_clock::now().time_since_epoch().count();
  }

  auto negative = is_set( "negative" );
  std::default_random_engine generator( seed );

  auto& circuits = env->store<circuit>();

  if ( is_set( "insert_gate" ) )
  {
    std::uniform_int_distribution<unsigned> dist( 0u, gates - 1u );
    create_random_gate( circuits.current().insert_gate( dist( generator ) ), lines, negative, generator );
  }
  else
  {
    circuit circ( lines );
    for ( auto i = 0u; i < gates; ++i )
    {
      create_random_gate( circ.append_gate(), lines, negative, generator );
    }

    if ( circuits.empty() || is_set( "new" ) )
    {
      circuits.extend();
    }
    circuits.current() = circ;
  }

  return true;
}

command::log_opt_t random_circuit_command::log() const
{
  return log_opt_t({
      {"lines", lines},
      {"gates", gates},
      {"negative", is_set( "negative" )},
      {"insert_gate", is_set( "insert_gate" )},
      {"seed", seed}
    });
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
