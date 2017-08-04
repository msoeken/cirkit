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

#include "simulate.hpp"

#include <iostream>
#include <map>
#include <vector>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/format.hpp>

#include <core/cli/stores.hpp>
#include <core/utils/program_options.hpp>
#include <classical/cli/stores.hpp>
#include <classical/functions/simulate_aig.hpp>
#include <classical/mig/mig_simulate.hpp>

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

simulate_command::simulate_command( const environment::ptr& env )
  : aig_mig_command( env, "Simulates an AIG", "Simulate current %s" )
{
  opts.add_options()
    ( "pattern,p",       value( &pattern ),    "Simulates an input pattern" )
    ( "assignment,s",    value( &assignment ), "Simulates an input assignment, e.g. \"x1=0 x2=1 x3=1 y=1010 z=01\"" )
    ( "tt,t",                                  "Simulates a truth table" )
    ( "bdd,b",                                 "Simulates a BDD" )
    ( "little_endian,l",                       "Change bit endianness to little-endian in assignment method (default: big-endian)" )
    ( "quiet,q",                               "Don't print simulation results" )
    ;

  if ( env->has_store<tt>() || env->has_store<bdd_function_t>() )
  {
    opts.add_options()
      ( "store,n", "Copy the result to the store (only for truth table and BDD simulation)" )
      ;
  }

  be_verbose();
}

command::rule_t simulate_command::one_simulation_method() const
{
  const auto assertion = [&]() {
    auto total = 0u;

    for ( const auto& o : {"pattern","assignment","tt","bdd"} )
    {
      if ( is_set( o ) ) { ++total; }
    }

    return total == 1u;
  };

  return {assertion, "exactly one simulation method needs to be chose"};
}

command::rule_t simulate_command::check_pattern_size() const
{
  const auto assertion = [&]() {
    if ( is_set( "pattern" ) )
    {
      if ( aig_selected() )
      {
        return pattern.size() == aig_info().inputs.size();
      }
      else if ( mig_selected() )
      {
        return pattern.size() == mig_info().inputs.size();
      }
    }

    return true;
  };
  return {assertion, "pattern has incorrect size"};
}

command::rules_t simulate_command::validity_rules() const
{
  auto rules = aig_mig_command::validity_rules();

  rules.push_back( one_simulation_method() );
  rules.push_back( check_pattern_size() );

  return rules;
}

bool simulate_command::execute_aig()
{
  tts.clear();

  if ( is_set( "pattern" ) )
  {
    simple_assignment_simulator::aig_name_value_map m;
    for ( auto i = 0u; i < aig_info().inputs.size(); ++i )
    {
      m.insert( { aig_info().node_names.at( aig_info().inputs.at( i ) ), pattern[i] == '1' } );
    }
    simple_assignment_simulator sim( m );

    properties_timer t( statistics );

    auto settings = make_settings();
    auto result = simulate_aig( aig(), sim, settings );

    for ( const auto& p : aig_info().outputs )
    {
      std::cout << boost::format( "[i] %s : %d" ) % p.second % result[p.first] << std::endl;
    }
  }
  else if ( is_set( "assignment" ) )
  {
    std::map<std::string, std::string> wassignment;
    std::vector<std::string> a;
    boost::split( a, assignment, boost::is_any_of( " " ), boost::token_compress_on );
    for ( const auto& p : a )
    {
      std::vector<std::string> vv;
      boost::split( vv, p, boost::is_any_of( "=" ) );
      assert( vv.size() == 2u );
      wassignment.insert( {vv[0u], vv[1u]} );
    }

    simple_assignment_simulator::aig_name_value_map massignment;
    for ( const auto& e : wassignment )
    {
      const unsigned size = e.second.size();
      if (size == 1u)
      {
        massignment.insert( {e.first, e.second[0u] == '1'} );
      }
      else
      {
        for ( auto u = 0u; u < size; ++u )
        {
          const std::string name = (boost::format("%s[%d]") % e.first % u).str();
          const bool value = (!is_set( "little_endian" ) ? e.second[size-u-1u] : e.second[u]) == '1';
          massignment.insert( {name, value} );
        }
      }
    }

    properties_timer t( statistics );

    auto values = simulate_aig( aig(), simple_assignment_simulator( massignment ) );

    for ( const auto& o : aig_info().outputs )
    {
      std::cout << boost::format( "[i] %s : %d" ) % o.second % values[o.first] << std::endl;
    }
  }
  else if ( is_set( "tt" ) )
  {
    properties_timer t( statistics );

    auto values = simulate_aig( aig(), tt_simulator() );

    for ( const auto& o : aig_info().outputs )
    {
      auto tt = values[o.first];

      if ( !is_set( "quiet" ) )
      {
        std::cout << boost::format( "[i] %s : " ) % o.second << tt << " (" << tt_to_hex( tt ) << ")" << std::endl;
      }
      store( tt );

      tts.push_back( to_string( tt ) );
    }
  }
  else if ( is_set( "bdd" ) )
  {
    properties_timer t( statistics );

    /* INFO: The bdd_simulator variable needs to be declared out of simulate_aig and
       also before the values map. It must be ensured that it is deleted after
       the values map or the BDDs in the values map are deleted, because they
       have a reference to the Cudd manager that is stored inside the bdd_simulator
       class. */
    Cudd mgr;
    bdd_simulator simulator( mgr );
    auto values = simulate_aig( aig(), simulator );

    std::vector<BDD> bdds;

    for ( const auto& o : aig_info().outputs )
    {
      if ( !is_set( "quiet" ) )
      {
        std::cout << boost::format( "[i] %s :" ) % o.second << std::endl;
        values[o.first].PrintMinterm();
      }

      bdds.push_back( values[o.first] );
    }

    store<bdd_function_t>( {mgr, bdds} );
  }

  if ( statistics->has_key( "runtime" ) )
  {
    print_runtime( statistics->get<double>( "runtime" ) );
  }

  return true;
}

bool simulate_command::execute_mig()
{
  tts.clear();

  if ( is_set( "pattern" ) )
  {
    mig_simple_assignment_simulator::mig_name_value_map m;
    for ( auto i = 0u; i < mig_info().inputs.size(); ++i )
    {
      m.insert( { mig_info().node_names.at( mig_info().inputs.at( i ) ), pattern[i] == '1' } );
    }
    mig_simple_assignment_simulator sim( m );

    auto settings = make_settings();
    auto result = simulate_mig( mig(), sim, settings );

    for ( const auto& p : mig_info().outputs )
    {
      std::cout << boost::format( "[i] %s : %d" ) % p.second % result[p.first] << std::endl;
    }
  }
  else if ( is_set( "tt" ) )
  {
    auto values = simulate_mig( mig(), mig_tt_simulator() );

    for ( const auto& o : mig_info().outputs )
    {
      auto tt = values[o.first];

      if ( mig_info().inputs.size() < tt_num_vars( tt ) )
      {
        tt_shrink( tt, mig_info().inputs.size() );
      }

      std::cout << boost::format( "[i] %s : " ) % o.second << tt << " (" << tt_to_hex( tt ) << ")" << std::endl;
      store( tt );

      tts.push_back( to_string( tt ) );
    }
  }

  return true;
}

command::log_opt_t simulate_command::log() const
{
  log_map_t m;

  if ( is_set( "tt" ) )
  {
    m["tts"] = tts;
  }

  if ( statistics->has_key( "runtime" ) )
  {
    m["runtime"] = statistics->get<double>( "runtime" );
  }

  if ( !m.empty() )
  {
    return m;
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
