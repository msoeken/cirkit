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

#include "satnpn.hpp"

#include <boost/format.hpp>

#include <core/utils/bitset_utils.hpp>
#include <core/utils/range_utils.hpp>
#include <classical/functions/aig_npn_canonization.hpp>

namespace cirkit
{

satnpn_command::satnpn_command( const environment::ptr& env )
  : aig_base_command( env, "Compute NPN class with SAT" )
{
  add_flag( "--progress,-p", "show progress" );
  add_option( "--miter,-m", miter, "miter:\n0: single miter\n1: shared miter", true );
  add_option( "--encoding,-e", encoding, "encoding:\n0: Tseytin\n2: EMS", true );
  add_option( "--heuristic", heuristic, "heuristic:\n0: flip-swap\n1: sifting", true );
  be_verbose();
  add_new_option();
}

void satnpn_command::execute()
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
}

nlohmann::json satnpn_command::log() const
{
  return nlohmann::json({
      {"miter", miter},
      {"encoding", encoding},
      {"heuristic", heuristic},
      {"runtime", statistics->get<double>( "runtime" )},
      {"sat_runtime", statistics->get<double>( "sat_runtime" )},
      {"miter_runtime", statistics->get<double>( "miter_runtime" )},
      {"encoding_runtime", statistics->get<double>( "encoding_runtime" )},
      {"lexsat_calls", statistics->get<unsigned long>( "lexsat_calls" )},
      {"sat_calls", statistics->get<unsigned long>( "sat_calls" )},
      {"num_inputs", statistics->get<std::vector<unsigned>>( "num_inputs" )}
    });
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
