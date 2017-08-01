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

#include "esopbs.hpp"

#include <boost/format.hpp>
#include <boost/program_options.hpp>

#include <alice/rules.hpp>
#include <classical/abc/gia/gia.hpp>
#include <classical/abc/gia/gia_esop.hpp>
#include <classical/cli/stores.hpp>
#include <classical/optimization/exorcism_minimization.hpp>
#include <classical/optimization/exorcism2.hpp>

#include <reversible/circuit.hpp>
#include <reversible/cli/stores.hpp>
#include <reversible/synthesis/esop_synthesis.hpp>

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

esopbs_command::esopbs_command( const environment::ptr& env )
  : cirkit_command( env, "ESOP based synthesis" )
{
  add_positional_option( "filename" );
  opts.add_options()
    ( "filename", value( &filename ), "filename to the ESOP file" )
    ( "mct",                          "no negative controls" )
    ( "no_shared_target",             "no shared target" )
    ( "no_constants",                 "no constant lines (but use PI for outputs)" )
    ( "aig,a",                        "read from AIG" )
    ( "exorcism,e",                   "use exorcism to optimize ESOP cover (only for --aig)" )
    ( "progress,p",                   "show progress" )
    ( "experimental",                 "experimental method for single-output AIGs" )
    ;
  add_new_option();
  be_verbose();
}

command::rules_t esopbs_command::validity_rules() const
{
  return {
    file_exists_if_set( *this, filename, "filename" ),
    has_store_element_if_set<aig_graph>( *this, env, "aig" )
  };
}

bool esopbs_command::execute()
{
  auto& circuits = env->store<circuit>();

  extend_if_new( circuits );

  auto settings = make_settings();
  settings->set( "progress", is_set( "progress" ) );

  if ( is_set( "filename" ) )
  {
    settings->set( "negative_control_lines", !is_set( "mct" ) );
    settings->set( "share_cube_on_target", !is_set( "no_shared_target" ) );
    esop_synthesis( circuits.current(), filename, settings, statistics );

    print_runtime();
  }
  else if ( is_set( "aig" ) )
  {
    const auto& aigs = env->store<aig_graph>();

    settings->set( "no_constants", is_set( "no_constants" ) );

    gia_graph gia( aigs.current() );
    auto esop = gia.compute_esop_cover( gia_graph::esop_cover_method::aig_new, settings );
    if ( is_set( "exorcism" ) )
    {
      esop = exorcism_minimization( esop, gia.num_inputs(), gia.num_outputs(), settings, statistics );
      print_runtime( "runtime", "exorcism" );
    }
    esop_synthesis( circuits.current(), esop, gia.num_inputs(), gia.num_outputs(), settings, statistics );

    print_runtime();
  }
  else if ( is_set( "experimental" ) )
  {
    const auto& aigs = env->store<aig_graph>();

    gia_graph gia( aigs.current() );
    const auto cubes = gia_extract_cover2( gia, settings );
    const auto cubes_opt = exorcism2( cubes, gia.num_inputs(), settings, statistics );
    print_runtime( "runtime", "exorcism" );

    esop_synthesis( circuits.current(), cubes_opt, gia.num_inputs(), settings, statistics );

    print_runtime();
  }

  return true;
}

command::log_opt_t esopbs_command::log() const
{
  return log_opt_t({
      {"mct", is_set( "mct" )},
      {"no_shared_target", is_set( "no_shared_target" )},
      {"runtime", statistics->get<double>( "runtime" )}
    });
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
