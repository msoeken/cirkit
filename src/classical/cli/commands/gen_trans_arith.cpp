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

#include "gen_trans_arith.hpp"

#include <fstream>

#include <boost/program_options.hpp>

#include <core/utils/program_options.hpp>
#include <core/utils/string_utils.hpp>
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
  opts.add_options()
    ( "filename",     value( &filename ),                  "filename" )
    ( "seed,s",       value( &seed ),                      "random seed (default: current time)" )
    ( "bitwidth,w",   value_with_default( &bitwidth ),     "bitwidth" )
    ( "min_words",    value_with_default( &min_words ),    "minimum number of input words" )
    ( "max_words",    value_with_default( &max_words ),    "maximum number of input words" )
    ( "max_fanout",   value_with_default( &max_fanout ),   "maximum number of fanout for each word" )
    ( "max_rounds",   value_with_default( &max_rounds ),   "maximum number of rounds, corresponds to levels" )
    ( "operators",    value_with_default( &operators ),    "space separated list of Verilog infix operators" )
    ( "word_pattern", value_with_default( &word_pattern ), "formatting for words" )
    ( "module_name",  value_with_default( &module_name ),  "name for resulting module" )
    ;
  add_positional_option( "filename" );
}

bool gen_trans_arith_command::execute()
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
  settings->set( "word_pattern", word_pattern );
  settings->set( "module_name", module_name );

  if ( !is_set( "filename" ) )
  {
    filename = "/dev/stdout";
  }

  std::ofstream os( filename.c_str(), std::ofstream::out );
  generate_transparent_arithmetic_circuit( os, settings, settings );
  os.close();

  return true;
}

command::log_opt_t gen_trans_arith_command::log() const
{
  return log_opt_t({
      {"seed", seed},
      {"min_words", min_words},
      {"max_words", max_words},
      {"max_fanout", max_fanout},
      {"max_rounds", max_rounds},
      {"operators", operators},
      {"word_pattern", word_pattern},
      {"module_name", module_name}
    });
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
