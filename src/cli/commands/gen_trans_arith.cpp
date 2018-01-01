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

#include "gen_trans_arith.hpp"

#include <fstream>

#include <boost/program_options.hpp>

#include <core/utils/program_options.hpp>
#include <core/utils/string_utils.hpp>
#include <cli/stores.hpp>
#include <classical/io/read_verilog.hpp>
#include <classical/generators/transparent_arithmetic.hpp>

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

gen_trans_arith_command::gen_trans_arith_command( const environment::ptr& env )
  : cirkit_command( env, "Generate transparent arithmetic circuits" )
{
  add_option( "--filename,filename", filename, "filename" );
  add_option( "--seed,-s", seed, "random seed (default: current time)" );
  add_option( "--bitwidth,-w",     bitwidth, "bitwidth", true );
  add_option( "--min_words",       min_words, "minimum number of input words", true );
  add_option( "--max_words",       max_words, "maximum number of input words", true );
  add_option( "--max_fanout",      max_fanout, "maximum number of fanout for each word", true );
  add_option( "--max_rounds",      max_rounds, "maximum number of rounds, corresponds to levels", true );
  add_option( "--operators",       operators, "space separated list of Verilog infix operators", true );
  add_option( "--mux_types",       mux_types, "mux types (M: standard multiplexer, O: one-hot transparencies)", true );
  add_option( "--mux_prob",        mux_prob, "probability of using a mux instead of operator (between 0 and 100)", true );
  add_option( "--new_ctrl_prob",   new_ctrl_prob, "probability of creating a new control input instead of using an existing one (between 0 and 100)", true );
  add_option( "--word_pattern",    word_pattern, "formatting for words", true );
  add_option( "--control_pattern", control_pattern, "formatting for control inputs", true );
  add_option( "--module_name",     module_name, "name for resulting module", true );
  add_flag( "--aig,-a", "write into AIG directly (don't specify filename)" );
  add_new_option();
}

command::rules_t gen_trans_arith_command::validity_rules() const
{
  return {
    {[this]() { return mux_prob <= 100u; }, "mux_prob must be between 0 and 100" },
    {[this]() { return new_ctrl_prob <= 100u; }, "new_ctrl_prob must be between 0 and 100" },
    {[this]() { return !is_set( "aig" ) || !is_set( filename ); }, "filename must be unset if writing into AIG" },
    {[this]() { return !operators.empty(); }, "operators cannot be empty" },
    {[this]() { return !mux_types.empty(); }, "mux_types cannot be empty" },
    {[this]() {
        for ( auto mt : mux_types )
        {
          if ( mt != 'M' && mt != 'O' )
          {
            return false;
          }
        }
        return true;
      }, "unsupported mux type" }
  };
}

void gen_trans_arith_command::execute()
{
  auto settings = make_settings();

  if ( is_set( "seed" ) )
  {
    settings->set( "seed", seed );
  }

  settings->set( "bitwidth", bitwidth );
  settings->set( "min_words", min_words );
  settings->set( "max_words", max_words );
  settings->set( "max_fanout", max_fanout );
  settings->set( "max_rounds", max_rounds );

  std::vector<std::string> voperators;
  split_string( voperators, operators, " " );
  settings->set( "operators", voperators );
  settings->set( "mux_types", mux_types );
  settings->set( "mux_prob", mux_prob );
  settings->set( "new_ctrl_prob", new_ctrl_prob );
  settings->set( "word_pattern", word_pattern );
  settings->set( "control_pattern", control_pattern );
  settings->set( "module_name", module_name );

  if ( is_set( "aig" ) )
  {
    filename = "/tmp/test.v";
  }
  else if ( !is_set( "filename" ) )
  {
    filename = "/dev/stdout";
  }

  std::ofstream os( filename.c_str(), std::ofstream::out );
  generate_transparent_arithmetic_circuit( os, settings, settings );
  os.close();

  if ( is_set( "aig" ) )
  {
    auto& aigs = env->store<aig_graph>();
    extend_if_new( aigs );
    aigs.current() = read_verilog_with_abc( filename );
  }
}

nlohmann::json gen_trans_arith_command::log() const
{
  return nlohmann::json({
      {"seed", seed},
      {"min_words", min_words},
      {"max_words", max_words},
      {"max_fanout", max_fanout},
      {"max_rounds", max_rounds},
      {"operators", operators},
      {"mux_types", mux_types},
      {"mux_prob", mux_prob},
      {"new_ctrl_prob", new_ctrl_prob},
      {"word_pattern", word_pattern},
      {"control_pattern", control_pattern},
      {"module_name", module_name}
    });
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
