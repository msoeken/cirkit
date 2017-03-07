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

#include "revsim.hpp"

#include <boost/dynamic_bitset.hpp>
#include <boost/program_options.hpp>

#include <alice/rules.hpp>
#include <reversible/cli/stores.hpp>
#include <reversible/simulation/partial_simulation.hpp>
#include <reversible/simulation/simple_simulation.hpp>

using namespace boost::program_options;

namespace cirkit
{

revsim_command::revsim_command( const environment::ptr& env )
  : cirkit_command( env, "Reversible circuit simulation" )
{
  opts.add_options()
    ( "partial,r",                    "use partial simulation" )
    ( "pattern,p", value( &pattern ), "simulation pattern" )
    ;
  add_positional_option( "pattern" );
}

command::rules_t revsim_command::validity_rules() const
{
  return {
    has_store_element<circuit>( env ),
    {[this]() {
        /* ext. pattern? */
        if ( pattern == "0*" || pattern == "1*") { return true; }

        for ( auto c : pattern )
        {
          if ( c != '0' && c != '1' )
          {
            return false;
          }
        }
        return true;
      }, "pattern must consists of 0s and 1s" },
    {[this]() { return pattern == "0*" ||
                       pattern == "1*" ||
                       ( is_set( "partial" ) || env->store<circuit>().current().lines() == pattern.size() ); }, "pattern bits must equal number of lines" }
  };
}

bool revsim_command::execute()
{
  const auto& circuits = env->store<circuit>();

  /* prepare pattern */
  if ( pattern == "0*" || pattern == "1*" )
  {
    pattern = std::string( circuits.current().lines(), pattern[0u] );
  }

  boost::dynamic_bitset<> input( pattern );
  boost::dynamic_bitset<> output;

  if ( is_set( "partial" ) )
  {
    partial_simulation( output, circuits.current(), input );
  }
  else
  {
    simple_simulation( output, circuits.current(), input );
  }

  std::cout << "[i] result: " << output << std::endl;

  return true;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
