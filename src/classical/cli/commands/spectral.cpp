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

#include "spectral.hpp"

#include <alice/rules.hpp>
#include <core/cli/stores.hpp>
#include <classical/cli/stores.hpp>
#include <classical/functions/spectral_canonization.hpp>
#include <classical/functions/spectral_canonization2.hpp>

namespace cirkit
{

spectral_command::spectral_command( const environment::ptr& env )
  : cirkit_command( env, "Spectral classification" )
{
  opts.add_options()
    ( "verify", "verify spectral transformation (for debugging)" )
    ;
  be_verbose();
  add_new_option();
}

command::rules_t spectral_command::validity_rules() const
{
  return {has_store_element<tt>( env )};
}

bool spectral_command::execute()
{
  auto& tts = env->store<tt>();

  spectral_class = get_spectral_class( tts.current() );

  if ( is_verbose() )
  {
    std::cout << "[i] class: " << spectral_class << std::endl;
  }

  spectral_normalization_params params;
  spectral_normalization_stats stats;

  params.verbose = is_verbose();
  params.verify = is_set( "verify" );

  specf = spectral_normalization( tts.current(), params, stats );

  extend_if_new( tts );
  tts.current() = specf;

  return true;
}

command::log_opt_t spectral_command::log() const
{
  return log_map_t( {
      {"tt", to_string( specf )},
      {"class", spectral_class}
    } );
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
