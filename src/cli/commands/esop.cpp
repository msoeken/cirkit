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

#include "esop.hpp"

#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>

#include <core/utils/program_options.hpp>
#include <core/utils/timer.hpp>
#include <classical/optimization/exorcism_minimization.hpp>

namespace cirkit
{

using boost::program_options::value;

esop_command::esop_command( const environment::ptr& env )
  : aig_base_command( env, "Generate ESOPs from AIGs" )
{
  opts.add_options()
    ( "filename",   value( &filename ),              "ESOP filename" )
    ( "collapse,c", value_with_default( &collapse ), "collapsing method:\naig (0): ABC's AIG collapsing\nbdd (1): PSDKRO collapsing\naignew (2): CirKit's AIG collapsing" )
    ( "minimize,m", value_with_default( &minimize ), "minimization method:\n0: none\n1: exorcism" )
    ( "progress,p",                                  "show progress" )
    ;
  add_new_option();
  add_positional_option( "filename" );
}

command::rules_t esop_command::validity_rules() const
{
  return {
    has_store_element<aig_graph>( env ),
    {[this]() { return is_set( "filename" ); }, "filename must be set"},
    {[this]() { return minimize <= 2u; }, "invalid value for minimize"},
    {[this]() { return ( collapse != gia_graph::esop_cover_method::bdd ) || ( info().outputs.size() == 1u ); }, "selected collapsing method can only be applied to single-output functions"}
  };
}

bool esop_command::execute()
{
  const auto settings = make_settings();
  settings->set( "progress", is_set( "progress" ) );

  gia_graph gia( aig() );

  auto esop = [&]() {
    reference_timer t( &collapse_runtime );
    return gia.compute_esop_cover( collapse, settings );
  }();

  switch ( minimize )
  {
  case 1u:
    esop = exorcism_minimization( esop, gia.num_inputs(), gia.num_outputs(), settings );
    break;
  }

  write_esop( esop, gia.num_inputs(), gia.num_outputs(), filename );

  return true;
}

command::log_opt_t esop_command::log() const
{
  return log_map_t({
      {"collapse", boost::lexical_cast<std::string>( collapse )},
      {"collapse_runtime", collapse_runtime},
      {"minimize", minimize}
    });
}


}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
