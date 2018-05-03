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

#include "exorcism.hpp"

#include <alice/rules.hpp>
#include <classical/abc/gia/gia.hpp>
#include <cli/stores.hpp>
#include <classical/optimization/exorcismq.hpp>
#include <classical/optimization/exorcism_minimization.hpp>

namespace cirkit
{

exorcism_command::exorcism_command( const environment::ptr& env )
  : cirkit_command( env, "Exorcism ESOP minimization" )
{
  add_option( "--filename,filename", filename, "ESOP filename for output" );
  add_option( "--aig,-a", "read from AIG" );
  add_option( "--psdkro,-p", "extract cover with PSDKROs (only for AIGs)" );
  be_verbose();
}

command::rules exorcism_command::validity_rules() const
{
  return {
    has_store_element_if_set<aig_graph>( *this, env, "aig" )
  };
}

void exorcism_command::execute()
{
  std::cout << "[w] deprecated: use command `esop' instead" << std::endl;

  const auto settings = make_settings();
  settings->set( "esopname", filename );
  settings->set( "skip_parsing", true );
  if ( is_set( "psdkro" ) )
  {
    settings->set( "cover_method", gia_graph::esop_cover_method::bdd );
  }

  const auto& aigs = env->store<aig_graph>();
  gia_graph gia( aigs.current() );
  const auto esop = exorcism_minimization( gia, settings, statistics );
  write_esop( esop, gia.num_inputs(), gia.num_outputs(), filename );

  print_runtime();
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
