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

#include "tt.hpp"

#include <fstream>

#include <boost/dynamic_bitset.hpp>

#include <core/utils/bitset_utils.hpp>
#include <core/utils/program_options.hpp>
#include <classical/cli/stores.hpp>

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

tt_command::tt_command( const environment::ptr& env )
  : command( env, "Truth table manipulation" ),
    tts( env->store<tt>() )
{
  add_positional_option( "load" );
  opts.add_options()
    ( "load,l",    value( &load ),    "Load a truth table into the store" )
    ( "planame,p", value( &planame ), "Write truth table to PLA" )
    ( "extend,e",  value( &extend ),  "Extend to bits" )
    ;
}

command::rules_t tt_command::validity_rules() const
{
  return {
    { [this]() { return is_set( "load" ) || tts.current_index() >= 0; }, "no current truth table available" },
    { [this]() { return static_cast<int>( is_set( "load" ) ) +
                        static_cast<int>( is_set( "planame" ) ) +
                        static_cast<int>( is_set( "extend" ) ) == 1; }, "only one option at a time" }
  };
}

bool tt_command::execute()
{
  if ( is_set( "load" ) )
  {
    tts.extend();
    tts.current() = boost::dynamic_bitset<>( load );
  }
  else if ( is_set( "planame" ) )
  {
    std::ofstream out( planame.c_str(), std::ofstream::out );

    out << ".i " << tt_num_vars( tts.current() ) << std::endl
        << ".o 1" << std::endl;

    auto index = 0u;
    boost::dynamic_bitset<> input( tt_num_vars( tts.current() ) );

    do {
      if ( tts.current().test( index ) )
      {
        out << input << " 1" << std::endl;
      }

      inc( input );
      ++index;
    } while ( input.any() );

    out << ".e" << std::endl;
  }
  else if ( is_set( "extend" ) )
  {
    tt_extend( tts.current(), extend );
  }

  return true;
}

command::log_opt_t tt_command::log() const
{
  if ( is_set( "load" ) )
  {
    return log_opt_t( {{"tt", to_string( tts.current() )}} );
  }
  else
  {
    return boost::none;
  }
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
