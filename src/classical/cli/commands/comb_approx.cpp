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

#include "comb_approx.hpp"

#include <boost/format.hpp>

#include <alice/rules.hpp>

#include <core/cli/stores.hpp>
#include <core/utils/bdd_utils.hpp>
#include <core/utils/program_options.hpp>
#include <core/utils/range_utils.hpp>
#include <classical/aig.hpp>
#include <classical/approximate/bdd_level_approximation.hpp>
#include <classical/approximate/error_metrics.hpp>
#include <classical/cli/stores.hpp>
#include <classical/dd/aig_from_cirkit_bdd.hpp>
#include <classical/dd/aig_to_cirkit_bdd.hpp>
#include <classical/dd/bdd.hpp>
#include <classical/dd/bdd_to_truth_table.hpp>
#include <classical/dd/size.hpp>
#include <classical/dd/visit_solutions.hpp>
#include <classical/functions/simulate_aig.hpp>

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

comb_approx_command::comb_approx_command( const environment::ptr& env )
  : cirkit_command( env, "Approximate combinational circuits" )
{
  opts.add_options()
    ( "bdd,b",          "Read and write from BDD" )
    ( "aig,a",          "Read and write from AIG" )
    ( "mode,m",         value_with_default( &mode ),           "Approximation mode:\n0: round-down\n1: round-up\n2: round-closest\n3: co-factor 0\n4: co-factor 1\n5: copy" )
    ( "level,l",        value_with_default( &level ),          "Round or co-factor at level (round is inclusive)" )
    ( "maximum_method", value_with_default( &maximum_method ), "Maximum method:\n0: shift\n1: chi" )
    ( "print,p",                                               "Print implicants of both functions" )
    ( "truthtable,t",                                          "Print truth table of both functions" )
    ( "new,n",                                                 "Create new store element for result" )
    ;
  be_verbose();
}

command::rules_t comb_approx_command::validity_rules() const
{
  return {
    {[this]() { return static_cast<int>( is_set( "bdd" ) ) + static_cast<int>( is_set( "aig" ) ) == 1; }, "either BDD or AIG needs to be chosen" },
    {[this]() { return !is_set( "bdd" ) || env->store<bdd_function_t>().current_index() >= 0; }, "no BDD in store" },
    {[this]() { return !is_set( "aig" ) || env->store<aig_graph>().current_index() >= 0; }, "no AIG in store" },
    {[this]() { return mode <= 5u; }, "mode needs to be at most 5" },
    {[this]() { return maximum_method <= 1u; }, "maximum method needs to be at most 1" }
  };
}

bool comb_approx_command::execute()
{
  using boost::format;

  auto& aigs = env->store<aig_graph>();
  // auto& bdds = env->store<bdd_function_t>();

  bdd_manager_ptr  manager;
  std::vector<bdd> fs;

  /* read from AIG or BDD */
  if ( is_set( "aig" ) )
  {
    cirkit_bdd_simulator sim( aigs.current(), 24u );
    auto map = simulate_aig( aigs.current(), sim );
    manager = sim.mgr;

    for ( const auto& m : map )
    {
      fs += m.second;
    }
  }
  else if ( is_set( "bdd" ) )
  {
    std::cerr << "[e] not implemented yet, use approximate_bdd program" << std::endl;
    return true;
  }
  else
  {
    assert( false );
  }

  if ( is_set( "verbose" ) )
  {
    std::cout << "[i] num_inputs:  " << manager->num_vars() << std::endl
              << "[i] num_outputs: " << fs.size() << std::endl;
  }

  if ( level > manager->num_vars() )
  {
    std::cerr << "[e] invalid level (must be less or equal to " << manager->num_vars() << ")" << std::endl;
    return true;
  }

  auto settings = std::make_shared<properties>();
  auto statistics = std::make_shared<properties>();

  auto fshat = ( mode == 5u ) ? fs : bdd_level_approximation( fs, (bdd_level_approximation_mode)mode, level, settings, statistics );

  /* print? */
  if ( is_set( "print" ) )
  {
    std::cout << "[i] Original function:" << std::endl;
    print_paths( fs );
    std::cout << "[i] Approximated function:" << std::endl;
    print_paths( fshat );
  }

  /* truth table? */
  auto metric_settings = std::make_shared<properties>();
  if ( is_set( "truthtable" ) )
  {
    std::cout << "[i] Original function:" << std::endl;
    for ( const auto& f : fs )
    {
      std::cout << bdd_to_truth_table( f ) << std::endl;
    }
    std::cout << "[i] Approximated function:" << std::endl;
    for ( const auto& fhat : fshat )
    {
      std::cout << bdd_to_truth_table( fhat ) << std::endl;
    }

    metric_settings->set( "print_truthtables", true );
  }

  /* write */
  if ( is_set( "aig" ) )
  {
    const auto& info = aig_info( aigs.current() );

    aig_graph aig;
    aig_initialize( aig );
    auto functions = aig_from_bdd( aig, fshat, settings );
    assert ( functions.size() == fshat.size() );
    for ( const auto& func : index( functions ) )
    {
      aig_create_po( aig, func.value, info.outputs[func.index].second );
    }

    if ( aigs.empty() || is_set( "new" ) )
    {
      aigs.extend();
    }
    aigs.current() = aig;
  }
  else if ( is_set( "bdd" ) )
  {
    assert( false );
  }
  else
  {
    assert( false );
  }

  /* statistics */
  auto er_settings   = std::make_shared<properties>();
  auto er_statistics = std::make_shared<properties>();
  auto wc_statistics = std::make_shared<properties>();
  auto ac_statistics = std::make_shared<properties>();

  auto er       = error_rate( fs, fshat, er_settings, er_statistics );
  auto size     = dd_size( fs );
  auto size_hat = dd_size( fshat );

  metric_settings->set( "maximum_method", static_cast<worst_case_maximum_method>( maximum_method ) );

  if ( mode < 5u )
  {
    std::cout << format( "[i] run-time:        %.2f secs" ) % statistics->get<double>( "runtime" ) << std::endl;
  }
  std::cout << "[i] old size:        " << size << std::endl;
  std::cout << format( "[i] new size:        %d (%.2f %%)" ) % size_hat % ( ( size - size_hat ) * 100.0 / size ) << std::endl;
  std::cout << format( "[i] error rate:      %d (%.2f %%)" ) % er % ( (double)er / (1ull << manager->num_vars()) * 100.0 ) << std::endl;
  std::cout << "[i] worst case:      " << worst_case( fs, fshat, metric_settings, wc_statistics ) << std::endl;
  std::cout << format( "[i] average case:    %.2f" ) % average_case( fs, fshat, metric_settings, ac_statistics ) << std::endl;

  std::cout << format( "[i] run-time (er):   %.2f" ) % er_statistics->get<double>( "runtime" ) << std::endl;
  std::cout << format( "[i] run-time (wc):   %.2f" ) % wc_statistics->get<double>( "runtime" ) << std::endl;
  std::cout << format( "[i] run-time (ac):   %.2f" ) % ac_statistics->get<double>( "runtime" ) << std::endl;

  return true;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
