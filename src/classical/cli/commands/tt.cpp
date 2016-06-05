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
#include <core/utils/string_utils.hpp>
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
  : cirkit_command( env, "Truth table manipulation" )
{
  add_positional_option( "load" );
  opts.add_options()
    ( "load,l",   value( &load ),   "load a truth table into the store" )
    ( "random,r", value( &random ), "create random truth table for number of variables" )
    ( "hwb",      value( &hwb ),    "create hwb function for number of bits" )
    ( "extend,e", value( &extend ), "extend to bits" )
    ( "swap,s",   value( &swap ),   "swaps to variables (seperated with comma, e.g., 2,3)" )
    ;
}

command::rules_t tt_command::validity_rules() const
{
  return {
    { [this]() { return is_set( "load" ) || is_set( "random" ) || is_set( "hwb" ) || env->store<tt>().current_index() >= 0; }, "no current truth table available" },
    { [this]() { return static_cast<int>( is_set( "load" ) ) +
                        static_cast<int>( is_set( "extend" ) ) +
                        static_cast<int>( is_set( "swap" ) ) +
                        static_cast<int>( is_set( "random" ) ) +
                        static_cast<int>( is_set( "hwb" ) ) == 1; }, "only one option at a time" }
  };
}

bool tt_command::execute()
{
  auto& tts = env->store<tt>();

  if ( is_set( "load" ) )
  {
    tts.extend();
    tts.current() = boost::dynamic_bitset<>( load );
  }
  else if ( is_set( "extend" ) )
  {
    tt_extend( tts.current(), extend );
  }
  else if ( is_set( "random" ) )
  {
    tts.extend();
    tts.current() = random_bitset( 1u << random );
  }
  else if ( is_set( "hwb" ) )
  {
    tt h( 1u << hwb );
    boost::dynamic_bitset<> it( hwb, 1 );

    do
    {
      h[it.to_ulong()] = it[it.count() - 1u];
      inc( it );
    } while ( it.any() );

    tts.extend();
    tts.current() = h;
  }
  else if ( is_set( "swap" ) )
  {
    const auto p = split_string_pair( swap, "," );
    tts.current() = tt_permute( tts.current(), boost::lexical_cast<unsigned>( p.first ), boost::lexical_cast<unsigned>( p.second ) );
  }

  return true;
}

command::log_opt_t tt_command::log() const
{
  if ( is_set( "load" ) || is_set( "random" ) )
  {
    return log_opt_t( {{"tt", to_string( env->store<tt>().current() )}} );
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
