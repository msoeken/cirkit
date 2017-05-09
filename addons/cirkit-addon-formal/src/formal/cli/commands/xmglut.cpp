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

#include "xmglut.hpp"

#include <boost/optional.hpp>
#include <boost/program_options.hpp>

#include <alice/rules.hpp>
#include <core/utils/program_options.hpp>
#include <classical/cli/stores.hpp>
#include <classical/netlist_graphs.hpp>
#include <classical/io/read_bench.hpp>
#include <classical/io/read_blif.hpp>
#include <classical/io/write_bench.hpp>
#include <classical/utils/aig_utils.hpp>
#include <classical/xmg/xmg_flow_map.hpp>
#include <classical/xmg/xmg_lut.hpp>
#include <formal/xmg/xmg_from_lut.hpp>
#include <classical/abc/functions/abc_lut_mapping.hpp>
#include <classical/abc/utils/abc_run_command.hpp>

using boost::program_options::value;

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

xmglut_command::xmglut_command( const environment::ptr& env )
  : cirkit_command( env, "Create XMG with LUT mapping" )
{
  opts.add_options()
    ( "lut_size,k", value_with_default( &lut_size ), "LUT size" )
    ( "map_cmd",    value_with_default( &map_cmd ),  "ABC map command in &space, use %d as placeholder for the LUT size" )
    ( "timeout,t",  value( &timeout ),               "timeout in seconds (afterwards, heuristics are tried)" )
    ( "xmg,x",                                       "create cover from XMG instead of AIG" )
    ( "noxor",                                       "don't use XOR, only works with LUT sizes up to 4" )
    ( "blif_name",  value( &blif_name ),             "read cover from BLIF instead of AIG" )
    ( "dump_luts",  value( &dump_luts ),             "if not empty, all LUTs will be written to file without performing mapping" )
    ( "progress,p",                                  "show progress" )
    ;
  add_new_option();
  be_verbose();
}

command::rules_t xmglut_command::validity_rules() const
{
  return {
    {[this]() { return is_set( "blif_name" ) || is_set( "xmg" ) || env->store<aig_graph>().current_index() != -1; }, "no AIG in store" },
    {[this]() { return !is_set( "xmg" ) || env->store<xmg_graph>().current_index() != -1; }, "no XMG in store" },
    {[this]() { return !is_set( "noxor" ) || lut_size <= 4; }, "LUT size can be at most 4 if no XOR is allowed" },
    file_exists_if_set( *this, blif_name, "blif_name" )
  };
}

bool xmglut_command::execute()
{
  auto& aigs = env->store<aig_graph>();
  auto& xmgs = env->store<xmg_graph>();

  auto settings = make_settings();
  settings->set( "lut_size", lut_size );
  settings->set( "noxor", is_set( "noxor" ) );
  settings->set( "progress", is_set( "progress" ) );
  if ( is_set( "dump_luts" ) )
  {
    settings->set( "npn", false );
    settings->set( "dump_luts", dump_luts );
  }
  if ( is_set( "timeout" ) )
  {
    settings->set( "timeout", boost::optional<unsigned>( timeout ) );
  }

  lut_graph_t lut;
  if ( is_set( "xmg" ) )
  {
    xmg_flow_map( xmgs.current(), settings );
    lut = xmg_to_lut_graph( xmgs.current() );
  }
  else if ( is_set( "blif_name" ) )
  {
    lut = read_blif( blif_name );
  }
  else
  {
    abc_run_command_no_output( aigs.current(), boost::str( boost::format( map_cmd ) % lut_size ) + "; &put; short_names; write_bench /tmp/test2.bench" );
    read_bench( lut, "/tmp/test2.bench" );
    //write_bench( lut, "/tmp/test3.bench" );
    //const auto lut = abc_lut_mapping( aig(), lut_size, settings );
  }

  const auto xmg = xmg_from_lut_mapping( lut, settings, statistics );

  if ( !is_set( "dump_luts" ) )
  {
    extend_if_new( xmgs );
    xmgs.current() = xmg;

    if ( !is_set( "xmg" ) && !is_set( "blif_name" ) )
    {
      xmgs.current().set_name( aig_info( aigs.current() ).model_name );
    }
  }

  print_runtime();

  return true;
}

command::log_opt_t xmglut_command::log() const
{
  return log_opt_t({
      {"runtime", statistics->get<double>( "runtime" ) },
      {"lut_size", lut_size}
    });
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
