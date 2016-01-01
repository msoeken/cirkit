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

using npn_func_t = std::function<tt(const tt&, boost::dynamic_bitset<>&, std::vector<unsigned>&, const properties::ptr&, const properties::ptr&)>;

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
    ( "approach",     value( &approach ),   "0: Exact\n1: Heuristic (based on number of 1s)\n2: Heuristic (based on lexicographic order)" )
    ( "enumerate,m",  value( &enumerate ),  "Computes NPN classes for all functions with given number of variables" )
    ( "truthtable,t",                       "Computes NPN class for the current truth table in the store" )
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
        "no current truth table available" },
    {[&]() { return approach <= 2u; },
        "approach must be value from 0 to 2" }
  };
}

bool npn_command::execute()
{
  std::vector<npn_func_t> approaches{ &exact_npn_canonization, &npn_canonization, &npn_canonization2 };
  const auto& func = approaches[approach];

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

        const auto key = func( bs, phase, perm, properties::ptr(), properties::ptr() ).to_ulong();
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

    npn = func( tts.current(), phase, perm, properties::ptr(), statistics );

    std::cout << boost::format( "[i] run-time: %.f secs" ) % statistics->get<double>( "runtime" ) << std::endl;
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
    return log_opt_t({
        {"runtime", statistics->get<double>( "runtime" )},
        {"phase", to_string( phase )},
        {"perm", any_join( perm, " " )},
        {"npn", to_string( npn )}
      });
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
