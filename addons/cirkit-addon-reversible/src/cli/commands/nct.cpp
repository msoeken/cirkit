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

#include "nct.hpp"

#include <alice/rules.hpp>
#include <cli/reversible_stores.hpp>
#include <reversible/mapping/nct_mapping.hpp>

namespace cirkit
{

nct_command::nct_command( const environment::ptr& env )
  : cirkit_command( env, "NCT mapping" )
{
  add_option( "--controls_threshold,-t", controls_threshold, "do not decompose any gates with less controls", true );
  add_new_option();
  be_verbose();
}

command::rules nct_command::validity_rules() const
{
  return {
    has_store_element<circuit>( env ),
    {[this]() { return controls_threshold >= 2u; }, "controls threshold must be at least 2"}
  };
}

void nct_command::execute()
{
  auto& circuits = env->store<circuit>();

  auto settings = make_settings();
  settings->set( "controls_threshold", controls_threshold );
  auto mapped = nct_mapping( circuits.current(), settings, statistics );
  print_runtime();

  extend_if_new( circuits );

  circuits.current() = mapped;
}

nlohmann::json nct_command::log() const
{
  return nlohmann::json({{"runtime", statistics->get<double>( "runtime" )}});
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
