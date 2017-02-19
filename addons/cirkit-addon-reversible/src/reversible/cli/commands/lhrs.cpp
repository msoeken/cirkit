/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2016  EPFL
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

#include "lhrs.hpp"

#include <sys/types.h>
#include <unistd.h>

#include <boost/program_options.hpp>

#include <core/utils/program_options.hpp>
#include <core/utils/temporary_filename.hpp>
#include <classical/abc/utils/abc_run_command.hpp>
#include <classical/io/read_blif.hpp>
#include <reversible/cli/stores.hpp>
#include <reversible/synthesis/lut_based_synthesis.hpp>

using boost::program_options::value;

namespace cirkit
{

lhrs_command::lhrs_command( const environment::ptr& env )
  : aig_base_command( env, "LUT based synthesis" )
{
  opts.add_options()
    ( "cut_size,k",        value_with_default( &cut_size ),        "cut size" )
    ( "order_heuristic,o", value_with_default( &order_heuristic ), "order heuristic\n0: defer\n1: eager" )
    ( "lutdecomp,l",                                               "apply LUT decomposition technique where possible" )
    ( "progress,p",                                                "show progress" )
    ;
  be_verbose();
  add_new_option();
}

bool lhrs_command::execute()
{
  auto& circuits = env->store<circuit>();
  extend_if_new( circuits );

  const auto settings = make_settings();
  settings->set( "order_heuristic", order_heuristic == 0 ? std::string( "defer" ) : std::string( "eager" ) );
  settings->set( "lutdecomp", is_set( "lutdecomp" ) );
  settings->set( "progress", is_set( "progress" ) );

  lut_graph_t lut;
  {
    temporary_filename blifname( "/tmp/lhrs-%d.blif" );

    abc_run_command_no_output( aig(), boost::str( boost::format( "&if -K %d -a; &put; write_blif %s" ) % cut_size % blifname.name() ) );

    lut = read_blif( blifname.name(), true );
    lut_count = lut_graph_lut_count( lut );
  }

  circuit circ;
  lut_based_synthesis( circuits.current(), lut, settings, statistics );

  print_runtime();

  return true;
}

command::log_opt_t lhrs_command::log() const
{
  return log_opt_t({
      {"runtime", statistics->get<double>( "runtime" )},
      {"cut_size", cut_size},
      {"lut_count", lut_count},
      {"num_decomp_default", statistics->get<unsigned>( "num_decomp_default" )},
      {"num_decomp_lut", statistics->get<unsigned>( "num_decomp_lut" )},
      {"class_counter", statistics->get<std::vector<std::vector<unsigned>>>( "class_counter" )},
      {"class_runtime", statistics->get<double>( "class_runtime" )}
    });
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
