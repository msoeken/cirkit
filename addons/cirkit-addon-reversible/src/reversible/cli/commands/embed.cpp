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

#include "embed.hpp"

#include <boost/format.hpp>

#include <alice/rules.hpp>

#include <core/properties.hpp>
#include <core/cli/stores.hpp>
#include <classical/cli/stores.hpp>
#include <reversible/cli/stores.hpp>
#include <reversible/functions/calculate_additional_lines.hpp>
#include <reversible/synthesis/embed_bdd.hpp>
#include <reversible/synthesis/embed_truth_table.hpp>

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
    ;
  add_new_option();
  be_verbose();
}

command::rules_t embed_command::validity_rules() const
{
  return {
    has_store_element_if_set<bdd_function_t>( *this, env, "bdd" ),
    has_store_element_if_set<bdd_function_t>( *this, env, "only_lines" ),
    {[this]() { return is_set( "bdd" ) || env->store<tt>().current_index() >= 0; }, "no current truth table available"}
  };
}

bool embed_command::execute()
{
  const auto& bdds = env->store<bdd_function_t>();

  const auto settings = make_settings();

  if ( is_set( "only_lines" ) )
  {
    auto lines = calculate_additional_lines( bdds.current(), settings, statistics );

    std::cout << boost::format( "[i] required lines: %d" ) % lines << std::endl;
  }
  else if ( is_set( "bdd" ) )
  {
    auto& rcbdds = env->store<rcbdd>();

    extend_if_new( rcbdds );
    embed_bdd( rcbdds.current(), bdds.current(), settings, statistics );
  }
  else
  {
    auto& specs = env->store<binary_truth_table>();
    const auto& tts = env->store<tt>();

    extend_if_new( specs );
    embed_truth_table( specs.current(), tts.current(), settings, statistics );
  }

  print_runtime();

  return true;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
