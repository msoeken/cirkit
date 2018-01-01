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

#include "migfh.hpp"

#include <boost/format.hpp>

#include <classical/mig/mig_functional_hashing.hpp>

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

migfh_command::migfh_command( const environment::ptr& env ) : mig_base_command( env, "Functional hashing for MIGs" )
{
  add_option( "--mode", mode, "0: top-down\n1: bottom-up", true );
  add_flag( "--ffrs,-f", "only optimize inside FFRs" );
  add_flag( "--depth_heuristic", "preserve depth locally" );
  add_option( "--hash", hash, "hash table size for NPN caching", true );
  add_flag( "--progress,-p", "show progress" );
  add_option( "--max_candidates", max_candidates,  "max candidates (only bottom-up)", true );
  add_flag( "--allow_area_inc", "allow area increase for candidates (only bottom-up)" );
  add_flag( "--allow_depth_inc", "allow depth increase for candidates (only bottom-up)" );
  add_option( "--sort_area_first", sort_area_first, "sort candidates by area, then depth (only bottom-up)", true );
  be_verbose();
}

void migfh_command::execute()
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
}

nlohmann::json migfh_command::log() const
{
  return nlohmann::json({
      {"runtime", statistics->get<double>( "runtime" )},
      {"runtime_cuts", statistics->get<double>( "runtime_cut" )},
      {"runtime_npn", statistics->get<double>( "runtime_npn" )},
      {"runtime_ffr", statistics->get<double>( "runtime_ffr" )},
      {"cache_hit", statistics->get<unsigned long>( "cache_hit" )},
      {"cache_miss", statistics->get<unsigned long>( "cache_miss" )}
    });
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
