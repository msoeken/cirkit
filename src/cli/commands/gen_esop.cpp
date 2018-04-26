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

#include "gen_esop.hpp"

#include <chrono>
#include <random>

#include <cli/stores.hpp>

#include <fmt/format.h>
#include <kitty/kitty.hpp>

namespace cirkit
{

gen_esop_command::gen_esop_command( const environment::ptr& env )
  : cirkit_command( env, "Enumerate ESOPs" )
{
  add_option( "-n,--num_vars", num_vars, "number of variables", true );
  add_option( "-m,--max_cubes", max_cubes, "maximum number of cubes", true );
  add_option( "-d,--min_distance", min_distance, "minimum distance of cube pairs" );
  add_option( "--max_tries", max_tries, "maximum number of rounds to compute new cubes", true );
  add_option( "--seed", seed, "random seed (current time, if not set" );
  add_flag( "-t,--store_as_tt", "stores ESOP function in truth table store" );
  be_verbose();
}

void gen_esop_command::execute()
{
  if ( !is_set( "--seed" ) )
  {
    seed = std::chrono::system_clock::now().time_since_epoch().count();
  }
  std::default_random_engine gen( seed );
  std::uniform_int_distribution<uint32_t> dist( 0, ( uint32_t( 1 ) << num_vars ) - 1 );

  std::vector<kitty::cube> cubes;
  
  for ( auto i = 0; i < max_tries; ++i )
  {
    const auto mask = dist( gen );
    const auto bits = dist( gen ) & mask;

    kitty::cube new_cube( bits, mask );

    const auto it = std::find_if( cubes.begin(), cubes.end(),
                                  [&new_cube, this]( const auto& cube ) { return cube.distance( new_cube ) < min_distance; } );
    if ( it == cubes.end() )
    {
      cubes.push_back( new_cube );
      if ( cubes.size() == max_cubes )
      {
        break;
      }
    } 
  }

  num_cubes = cubes.size();

  if ( is_verbose() )
  {
    env->out() << fmt::format( "found ESOP with {} cubes\n", cubes.size() );
    for ( const auto& c : cubes )
    {
      c.print( num_vars, env->out() );
      env->out() << '\n';
    }

    env->out() << std::flush;
  }

  if ( is_set( "--store_as_tt" ) )
  {
    kitty::dynamic_truth_table table( num_vars );
    kitty::create_from_cubes( table, cubes, true );
    store<tt>().extend();
    store<tt>().current() = from_kitty( table );
  }
}

nlohmann::json gen_esop_command::log() const
{
  return nlohmann::json({
    {"num_vars", num_vars},
    {"num_vars", num_vars},
    {"max_cubes", max_cubes},
    {"min_distance", min_distance},
    {"max_tries", max_tries},
    {"seed", seed},
    {"num_cubes", num_cubes}
  });
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
