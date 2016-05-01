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

#include "satnpn.hpp"

#include <boost/format.hpp>

#include <core/utils/bitset_utils.hpp>
#include <core/utils/program_options.hpp>
#include <core/utils/range_utils.hpp>
#include <formal/functions/aig_npn_canonization.hpp>

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


satnpn_command::satnpn_command( const environment::ptr& env )
  : aig_base_command( env, "Compute NPN class with SAT" )
{
  opts.add_options()
    ( "new,n",                                        "Add result into new store entry" )
    ( "progress,p",                                   "Show progress" )
    ( "miter,m",    value_with_default( &miter ),     "Miter:\n0: single miter\n1: shared miter" )
    ( "encoding,e", value_with_default( &encoding ),  "Encoding:\n0: Tseytin\n2: EMS" )
    ( "heuristic",  value_with_default( &heuristic ), "Heuristic:\n0: flip-swap\n1: sifting" )
    ;
  be_verbose();
}

bool satnpn_command::execute()
{
  const auto aig_current = aig();

  if ( is_set( "new" ) )
  {
    env->store<aig_graph>().extend();
  }

  const auto settings = make_settings();
  settings->set( "progress", is_set( "progress" ) );
  settings->set( "miter", miter );
  settings->set( "encoding", encoding );
  settings->set( "heuristic", heuristic );
  aig() = aig_npn_canonization( aig_current, settings, statistics );

  std::cout << boost::format( "[i] run-time:       %.2f secs\n[i] run-time "
                              "(SAT): %.2f secs\n[i] run-time (MIT): %.2f "
                              "secs\n[i] run-time (ENC): %.2f secs\n[i] SAT "
                              "calls:      %d\n[i] "
                              "LEXSAT calls:   %d" ) %
                   statistics->get<double>( "runtime" ) %
                   statistics->get<double>( "sat_runtime" ) %
                   statistics->get<double>( "miter_runtime" ) %
                   statistics->get<double>( "encoding_runtime" ) %
                   statistics->get<unsigned long>( "sat_calls" ) %
                   statistics->get<unsigned long>( "lexsat_calls" )
            << std::endl;

  return true;
}

command::log_opt_t satnpn_command::log() const
{
  return log_opt_t({
      {"miter", miter},
      {"encoding", encoding},
      {"heuristic", heuristic},
      {"runtime", statistics->get<double>( "runtime" )},
      {"sat_runtime", statistics->get<double>( "sat_runtime" )},
      {"miter_runtime", statistics->get<double>( "miter_runtime" )},
      {"encoding_runtime", statistics->get<double>( "encoding_runtime" )},
      {"lexsat_calls", static_cast<int>( statistics->get<unsigned long>( "lexsat_calls" ) )},
      {"sat_calls", static_cast<int>( statistics->get<unsigned long>( "sat_calls" ) )},
      {"num_inputs", statistics->get<std::vector<unsigned>>( "num_inputs" )}
    });
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
