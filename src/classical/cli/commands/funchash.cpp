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

#include "funchash.hpp"

#include <boost/format.hpp>

#include <core/utils/program_options.hpp>
#include <classical/mig/mig_functional_hashing.hpp>

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

funchash_command::funchash_command( const environment::ptr& env ) : mig_base_command( env, "Functional hashing for MIGs" )
{
  opts.add_options()
    ( "mode",            value_with_default( &mode ),            "0: top-down\n1: bottom-up" )
    ( "ffrs,f",                                                  "Only optimize inside FFRs" )
    ( "depth_heuristic",                                         "Preserve depth locally" )
    ( "hash",            value_with_default( &hash ),            "Hash table size for NPN caching" )
    ( "progress,p",                                              "Show progress" )
    ( "max_candidates",  value_with_default( &max_candidates ),  "Max candidates (only bottom-up)" )
    ( "allow_area_inc",                                          "Allow area increase for candidates (only bottom-up)" )
    ( "allow_depth_inc",                                         "Allow depth increase for candidates (only bottom-up)" )
    ( "sort_area_first", value_with_default( &sort_area_first ), "Sort candidates by area, then depth (only bottom-up)" )
    ;
  be_verbose();
}

bool funchash_command::execute()
{
  const auto settings = make_settings();
  settings->set( "top_down",            mode == 0u );
  settings->set( "use_ffrs",            is_set( "ffrs" ) );
  settings->set( "depth_heuristic",     is_set( "depth_heuristic" ) );
  settings->set( "npn_hash_table_size", hash );
  settings->set( "progress",            is_set( "progress" ) );
  settings->set( "max_candidates",      max_candidates );
  settings->set( "allow_area_inc",      is_set( "allow_area_inc" ) );
  settings->set( "allow_depth_inc",     is_set( "allow_depth_inc" ) );
  settings->set( "sort_area_first",     sort_area_first );
  mig() = mig_functional_hashing( mig(), settings, statistics );

  auto cache_hit  = statistics->get<unsigned long>( "cache_hit" );
  auto cache_miss = statistics->get<unsigned long>( "cache_miss" );

  auto hit_rate  = (double)cache_hit / ( cache_hit + cache_miss );
  auto miss_rate = 1.0 - hit_rate;

  std::cout << boost::format( "[i] run-time:        %.2f secs" ) % statistics->get<double>( "runtime" ) << std::endl
            << boost::format( "[i] run-time (cuts): %.2f secs" ) % statistics->get<double>( "runtime_cut" ) << std::endl
            << boost::format( "[i] run-time (NPN):  %.2f secs" ) % statistics->get<double>( "runtime_npn" ) << std::endl
            << boost::format( "[i] run-time (ffrs): %.2f secs" ) % statistics->get<double>( "runtime_ffr" ) << std::endl
            << boost::format( "[i] cache hit:       %u (%.2f %%)" ) % cache_hit % ( hit_rate * 100.0) << std::endl
            << boost::format( "[i] cache miss:      %u (%.2f %%)" ) % cache_miss % ( miss_rate * 100.0 ) << std::endl;

  return true;
}

command::log_opt_t funchash_command::log() const
{
  return log_opt_t({
      {"runtime", statistics->get<double>( "runtime" )},
      {"runtime_cuts", statistics->get<double>( "runtime_cut" )},
      {"runtime_npn", statistics->get<double>( "runtime_npn" )},
      {"runtime_ffr", statistics->get<double>( "runtime_ffr" )},
      {"cache_hit", static_cast<unsigned>( statistics->get<unsigned long>( "cache_hit" ) )},
      {"cache_miss", static_cast<unsigned>( statistics->get<unsigned long>( "cache_miss" ) )}
    });
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
