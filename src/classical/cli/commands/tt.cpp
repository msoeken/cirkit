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

#include "tt.hpp"

#include <fstream>

#include <boost/dynamic_bitset.hpp>
#include <boost/lexical_cast.hpp>

#include <core/utils/bitset_utils.hpp>
#include <core/utils/conversion_utils.hpp>
#include <core/utils/program_options.hpp>
#include <core/utils/string_utils.hpp>
#include <classical/cli/stores.hpp>

using namespace boost::program_options;

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

static constexpr unsigned primes[] = {
  2,      3,      5,      7,      11,     13,     17,     19,     23,     29,
  31,     37,     41,     43,     47,     53,     59,     61,     67,     71,
  73,     79,     83,     89,     97,     101,    103,    107,    109,    113,
  127,    131,    137,    139,    149,    151,    157,    163,    167,    173,
  179,    181,    191,    193,    197,    199,    211,    223,    227,    229,
  233,    239,    241,    251,    257,    263,    269,    271,    277,    281,
  283,    293,    307,    311,    313,    317,    331,    337,    347,    349,
  353,    359,    367,    373,    379,    383,    389,    397,    401,    409,
  419,    421,    431,    433,    439,    443,    449,    457,    461,    463,
  467,    479,    487,    491,    499,    503,    509,    521,    523,    541,
  547,    557,    563,    569,    571,    577,    587,    593,    599,    601,
  607,    613,    617,    619,    631,    641,    643,    647,    653,    659,
  661,    673,    677,    683,    691,    701,    709,    719,    727,    733,
  739,    743,    751,    757,    761,    769,    773,    787,    797,    809,
  811,    821,    823,    827,    829,    839,    853,    857,    859,    863,
  877,    881,    883,    887,    907,    911,    919,    929,    937,    941,
  947,    953,    967,    971,    977,    983,    991,    997,    1009,   1013,
  1019,   1021,   1024 /* last element is sentinel */
};

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
    ( "load,l",   value( &load ),   "load a truth table into the store (default is binary format, use 0x prefix for hexadecimal format)" )
    ( "random,r", value( &random ), "create random truth table for number of variables" )
    ( "hwb",      value( &hwb ),    "create hwb function for number of bits" )
    ( "maj",      value( &maj ),    "create maj function for number of odd bits" )
    ( "prime",    value( &prime ),  "create prime function that is true, whenever the input assignment is prime (for up to 10 bits)" )
    ( "extend,e", value( &extend ), "extend to bits" )
    ( "shrink",   value( &shrink ), "shrink to bits" )
    ( "swap,s",   value( &swap ),   "swaps to variables (seperated with comma, e.g., 2,3)" )
    ;
}

command::rules_t tt_command::validity_rules() const
{
  return {
    { [this]() { return is_set( "load" ) || is_set( "random" ) || is_set( "hwb" ) || is_set( "maj" ) || is_set( "prime" ) || env->store<tt>().current_index() >= 0; }, "no current truth table available" },
    { [this]() { return !is_set( "maj" ) || maj % 2 == 1; }, "argument to maj must be odd" },
    { [this]() { return !is_set( "prime" ) || prime <= 10u; }, "argument to prime cannot be larger than 10" },
    { [this]() { return static_cast<int>( is_set( "load" ) ) +
                        static_cast<int>( is_set( "extend" ) ) +
                        static_cast<int>( is_set( "shrink" ) ) +
                        static_cast<int>( is_set( "swap" ) ) +
                        static_cast<int>( is_set( "random" ) ) +
                        static_cast<int>( is_set( "hwb" ) ) +
                        static_cast<int>( is_set( "maj" ) ) +
                        static_cast<int>( is_set( "prime" ) ) == 1; }, "only one option at a time" }
  };
}

bool tt_command::execute()
{
  auto& tts = env->store<tt>();

  if ( is_set( "load" ) )
  {
    /* is load in hex? */
    if ( load.size() >= 2u && load[0] == '0' && load[1] == 'x' )
    {
      load = convert_hex2bin( load.substr( 2u ) );
    }
    else if ( load.size() >= 2u && load[0] == '0' && load[1] == 'd' )
    {
      const auto col = load.find(':');
      if ( col == std::string::npos )
      {
        return true;
      }

      const auto num_vars = boost::lexical_cast<unsigned>( load.substr( 2u, col - 2u ) );
      const auto func     = boost::lexical_cast<unsigned>( load.substr( col + 1 ) );

      load = to_string( boost::dynamic_bitset<>( 1 << num_vars, func ) );
    }

    tts.extend();
    tts.current() = boost::dynamic_bitset<>( load );
  }
  else if ( is_set( "extend" ) )
  {
    tt_extend( tts.current(), extend );
  }
  else if ( is_set( "shrink" ) )
  {
    tt_shrink( tts.current(), shrink );
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
  else if ( is_set( "maj" ) )
  {
    tt m( 1u << maj );
    boost::dynamic_bitset<> it( maj, 0 );

    do
    {
      m[it.to_ulong()] = it.count() > ( maj >> 1u );
      inc( it );
    } while ( it.any() );

    tts.extend();
    tts.current() = m;
  }
  else if ( is_set( "prime" ) )
  {
    tt p( 1u << prime );
    auto pt = primes;
    auto max = p.size();

    while ( *pt < max )
    {
      p.set( *pt++ );
    }

    tts.extend();
    tts.current() = p;
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
  if ( env->store<tt>().current_index() != -1 )
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
