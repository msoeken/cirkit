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

#include "gen_reciprocal.hpp"

#include <boost/program_options.hpp>

#include <core/utils/program_options.hpp>
#include <reversible/cli/stores.hpp>
#include <reversible/generators/reciprocal.hpp>
#include <reversible/verification/validate_reciprocal.hpp>

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
gen_reciprocal_command::gen_reciprocal_command( const environment::ptr& env )
  : cirkit_command( env, "Generate reciprocal circuit" )
{
  opts.add_options()
    ( "bitwidth,w",   value_with_default( &bitwidth ),     "bitwidth" )
    ( "algorithm,a",  value_with_default( &algorithm ),    "algorithm\n0: direct\n1: Newton" )
    ( "method,m",     value_with_default( &method ),       "synthesis method\n0: ESOP based\n1: PLA based" )
    ( "verilog_name", value_with_default( &verilog_name ), "filename for intermediate Verilog file" )
    ( "esop_name",    value_with_default( &esop_name ),    "filename for intermediate ESOP file" )
    ( "pla_name",     value_with_default( &pla_name ),     "filename for intermediate PLA file" )
    ( "only_write",                                        "only write Verilog file and stop" )
    ( "validate",                                          "validate resulting circuit" )
    ;
  boost::program_options::options_description newton_options( "Newton options" );
  newton_options.add_options()
    ( "iterations,i", value( &iterations ),                "number of iterations (for Newton algorithm)\nif not specified, it's automatically determined based on the bitwidth" )
    ( "blif_name",    value_with_default( &blif_name ),    "filename for intermediate BLIF file" )
    ;
  opts.add( newton_options );
  add_positional_option( "bitwidth" );
  be_verbose();
  add_new_option();
}

command::rules_t gen_reciprocal_command::validity_rules() const
{
  return {
    {[this]() { return algorithm <= 1u; }, "algorithm must be value from 0 to 1"},
    {[this]() { return method <= 1u; }, "method must be value from 0 to 1"}
  };
}

bool gen_reciprocal_command::execute()
{
  auto settings = make_settings();

  settings->set( "method", method );
  if ( is_set( "iterations" ) )
  {
    settings->set( "iterations", iterations );
  }
  settings->set( "verilog_name", verilog_name );
  settings->set( "esop_name", esop_name );
  settings->set( "pla_name", pla_name );
  settings->set( "blif_name", blif_name );
  settings->set( "only_write", is_set( "only_write" ) );

  circuit circ;

  switch ( algorithm )
  {
  default: assert( false );

  case 0u:
    circ = generate_reciprocal_direct( bitwidth, settings, statistics );
    break;
  case 1u:
    circ = generate_reciprocal_newton( bitwidth, settings, statistics );
    break;
  }

  print_runtime();

  if ( !is_set( "only_write" ) )
  {
    auto& circuits = env->store<circuit>();
    extend_if_new( circuits );
    circuits.current() = circ;

    if ( is_set( "validate" ) )
    {
      validate_reciprocal( circ, bitwidth, settings );
    }
  }


  return true;
}

command::log_opt_t gen_reciprocal_command::log() const
{
  return log_opt_t({
      {"runtime", statistics->get<double>( "runtime" )},
      {"bitwidth", bitwidth},
      {"algorithm", algorithm},
      {"method", method}
    });
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
