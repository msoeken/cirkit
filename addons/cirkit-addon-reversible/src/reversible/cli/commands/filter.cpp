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

#include "filter.hpp"

#include <alice/rules.hpp>

#include <functional>

#include <reversible/target_tags.hpp>
#include <reversible/cli/stores.hpp>
#include <reversible/functions/filter_circuit.hpp>

namespace cirkit
{

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

using gate_pred_t = std::function<bool( const gate& )>;
gate_pred_t make_or_filter( gate_pred_t f1, gate_pred_t f2 )
{
  return [&f1, &f2]( const gate& g ) {
    return f1( g ) || f2( g );
  };
}

gate_pred_t make_not_filter( gate_pred_t f1 )
{
  return [&f1]( const gate& g ) {
    return !f1( g );
  };
}

/******************************************************************************
 * Command                                                                    *
 ******************************************************************************/

filter_command::filter_command( const environment::ptr& env )
  : cirkit_command( env, "Filters a reversible circuit" )
{
  opts.add_options()
    ( "tof,t", "keep Toffoli gates" )
    ( "stg,s", "keep single-target gates" )
    ( "inv,i", "invert filter" )
    ;
  add_new_option();
}

command::rules_t filter_command::rules() const
{
  return {has_store_element<circuit>( env )};
}

bool filter_command::execute()
{
  auto& circuits = env->store<circuit>();

  const auto f_id = []( const gate& gate ) { return false; };
  auto filter = gate_pred_t( f_id );

  if ( is_set( "tof" ) )
  {
    filter = make_or_filter( filter, []( const gate& g ) { return is_toffoli( g ); } );
  }

  if ( is_set( "stg" ) )
  {
    filter = make_or_filter( filter, []( const gate& g ) { return is_stg( g ); } );
  }

  if ( is_set( "inv" ) )
  {
    filter = make_not_filter( filter );
  }

  const auto circ = filter_circuit( circuits.current(), filter );

  extend_if_new( circuits );
  circuits.current() = circ;

  return true;
}

command::log_opt_t filter_command::log() const
{
  return boost::none;
}


}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
