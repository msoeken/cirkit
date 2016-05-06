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

#include "embed.hpp"

#include <boost/format.hpp>

#include <lscli/rules.hpp>

#include <core/properties.hpp>
#include <core/cli/stores.hpp>

#include <reversible/cli/stores.hpp>
#include <reversible/functions/calculate_additional_lines.hpp>
#include <reversible/synthesis/embed_bdd.hpp>

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

embed_command::embed_command( const environment::ptr& env )
  : cirkit_command( env, "Embedding" )
{
  opts.add_options()
    ( "bdd,b",      "Embed from BDDs" )
    ( "only_lines", "Only calculate additional lines" )
    ( "new,n",      "Add a new entry to the store; if not set, the current entry is overriden" )
    ;
  be_verbose();
}

command::rules_t embed_command::validity_rules() const
{
  return { has_store_element<bdd_function_t>( env ) };
}

bool embed_command::execute()
{
  const auto& bdds = env->store<bdd_function_t>();
  auto& rcbdds = env->store<rcbdd>();

  const auto settings = make_settings();
  const auto statistics = std::make_shared<properties>();

  if ( is_set( "only_lines" ) )
  {
    auto lines = calculate_additional_lines( bdds.current(), settings, statistics );

    std::cout << boost::format( "[i] required lines: %d" ) % lines << std::endl;
  }
  else
  {
    if ( rcbdds.empty() || is_set( "new" ) )
    {
      rcbdds.extend();
    }

    embed_bdd( rcbdds.current(), bdds.current(), settings, statistics );
  }

  std::cout << boost::format( "[i] run-time: %.2f secs" ) % statistics->get<double>( "runtime" ) << std::endl;

  return true;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
