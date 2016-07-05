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

#include "revsim.hpp"

#include <boost/dynamic_bitset.hpp>
#include <boost/program_options.hpp>

#include <alice/rules.hpp>
#include <reversible/cli/stores.hpp>
#include <reversible/simulation/simple_simulation.hpp>

using namespace boost::program_options;

namespace cirkit
{

revsim_command::revsim_command( const environment::ptr& env )
  : cirkit_command( env, "Reversible circuit simulation" )
{
  opts.add_options()
    ( "pattern,p", value( &pattern ), "simulation pattern" )
    ;
  add_positional_option( "pattern" );
}

command::rules_t revsim_command::validity_rules() const
{
  return {
    has_store_element<circuit>( env ),
    {[this]() {
        for ( auto c : pattern )
        {
          if ( c != '0' && c != '1' )
          {
            return false;
          }
        }
        return true;
      }, "pattern must consists of 0s and 1s" },
    {[this]() { return env->store<circuit>().current().lines() == pattern.size(); }, "pattern bits must equal number of lines" }
  };
}

bool revsim_command::execute()
{
  const auto& circuits = env->store<circuit>();

  boost::dynamic_bitset<> input( pattern );
  boost::dynamic_bitset<> output;
  simple_simulation( output, circuits.current(), input );

  std::cout << "[i] result: " << output << std::endl;

  return true;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
