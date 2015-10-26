/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
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

#include "npn.hpp"

#include <fstream>
#include <functional>
#include <unordered_map>

#include <boost/dynamic_bitset.hpp>
#include <boost/format.hpp>

#include <core/utils/bitset_utils.hpp>
#include <core/utils/program_options.hpp>
#include <core/utils/range_utils.hpp>
#include <core/utils/timer.hpp>
#include <classical/cli/stores.hpp>
#include <classical/functions/npn_canonization.hpp>
#include <classical/utils/truth_table_utils.hpp>

using namespace boost::program_options;
using boost::format;

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

using npn_func_t = std::function<tt(const tt&, boost::dynamic_bitset<>&, std::vector<unsigned>&)>;

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

npn_command::npn_command( const environment::ptr& env )
  : command( env, "NPN classification" )
{
  opts.add_options()
    ( "enumerate,m",  value( &enumerate ),  "Computes NPN classes for all functions with given number of variables" )
    ( "truthtable,t",                       "Computes NPN class for the current truth table in the store" )
    ( "exact,e",                            "Use exact computation" )
    ( "logname,l",    value( &logname ),    "If enumerate is set, write all classes to this file" )
    ( "store,n",                            "Copy the result to the store (only for truth tables)" )
    ;
}

command::rules_t npn_command::validity_rules() const
{
  return {
    {[&]() { return !opts.is_set( "truthtable" ) || !opts.is_set( "enumerate" ); },
        "either truth table or enumeration can be performed" },
    {[&]() { return !opts.is_set( "truthtable" ) || env->store<tt>().current_index() >= 0; },
        "no current truth table available" }
  };
}

bool npn_command::execute()
{
  const auto func = opts.is_set( "exact" ) ? npn_func_t( &exact_npn_canonization ) : npn_func_t( &npn_canonization );

  if ( opts.is_set( "enumerate" ) )
  {
    double runtime;
    std::unordered_map<unsigned, unsigned> classes;

    {
      reference_timer t( &runtime );

      boost::dynamic_bitset<> bs( 1u << enumerate );

      boost::dynamic_bitset<> phase;
      std::vector<unsigned>   perm;

      do
      {
        phase.clear();
        perm.clear();

        const auto key = func( bs, phase, perm ).to_ulong();
        auto it = classes.find( key );
        if ( it == classes.end() )
        {
          classes.insert( {key, 1u} );
        }
        else
        {
          it->second++;
        }

        inc( bs );
      } while ( bs.any() );
    }

    std::cout << format( "[i] found %d classes in %.2f secs" ) % classes.size() % runtime << std::endl;

    if ( opts.is_set( "logname" ) )
    {
      std::ofstream out( logname.c_str(), std::ofstream::out );
      for ( const auto& value : classes )
      {
        boost::dynamic_bitset<> tt( 1u << enumerate, value.first );
        out << tt_to_hex( tt ) << " " << tt << " " << value.second << std::endl;
      }
    }
  }

  if ( opts.is_set( "truthtable" ) )
  {
    auto& tts = env->store<tt>();

    npn = func( tts.current(), phase, perm );

    std::cout << "[i] NPN class for " << tts.current() << " is " << npn << std::endl;
    std::cout << "[i] - phase: " << phase << " perm: " << any_join( perm, " " ) << std::endl;

    if ( opts.is_set( "store" ) )
    {
      tts.extend();
      tts.current() = npn;
    }
  }

  return true;
}

command::log_opt_t npn_command::log() const
{
  if ( opts.is_set( "truthtable" ) )
  {
    return log_opt_t( {{"phase", to_string( phase )}, {"perm", any_join( perm, " " )}, {"npn", to_string( npn )}} );
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
