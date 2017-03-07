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

/**
 * @author Mathias Soeken
 */

#include <fstream>

#include <boost/format.hpp>

#include <core/utils/program_options.hpp>
#include <core/utils/range_utils.hpp>

#include <classical/approximate/bdd_level_approximation.hpp>
#include <classical/approximate/error_metrics.hpp>
#include <classical/dd/bdd.hpp>
#include <classical/dd/bdd_to_truth_table.hpp>
#include <classical/dd/size.hpp>
#include <classical/dd/visit_solutions.hpp>
#include <classical/io/read_into_bdd.hpp>
#include <classical/io/write_from_bdd.hpp>

using namespace cirkit;

int main( int argc, char ** argv )
{
  using boost::format;
  using boost::str;
  using boost::program_options::value;

  std::string filename, outname;
  auto mode           = 0u;
  auto level          = 0u;
  auto maximum_method = 0u;

  program_options opts;
  opts.add_options()
    ( "filename",       value( &filename ),                    "Filename (either *.aag or *.pla file)" )
    ( "outname",        value( &outname ),                     "Output filename (eigher *.aag or *.pla file)" )
    ( "mode",           value_with_default( &mode ),           "Mode (0: round-down, 1: round-up, 2: round-closest, 3: co-factor 0, 4: co-factor 1, 5: copy)" )
    ( "level",          value_with_default( &level ),          "Round or co-factor at level (round is inclusive)" )
    ( "maximum_method", value_with_default( &maximum_method ), "Maximum method (0: shift, 1: chi)" )
    ( "print,p",                                               "Print implicants of both functions" )
    ( "truthtable,t",                                          "Print truth table of both functions" )
    ( "verbose,v",                                             "Be verbose" )
    ;
  opts.parse( argc, argv );

  if ( !opts.good() || !opts.is_set( "filename" ) || mode > 5u )
  {
    std::cout << opts << std::endl;
    return 1;
  }

  /* read BDD */
  auto rib_settings = std::make_shared<properties>();
  rib_settings->set( "verbose", opts.is_set( "verbose" ) );
  auto rib_statistics = std::make_shared<properties>();

  bdd_manager_ptr  manager;
  std::vector<bdd> fs;
  std::tie( manager, fs ) = read_into_bdd( filename, rib_settings, rib_statistics );

  if ( opts.is_set( "verbose" ) )
  {
    std::cout << "[i] num_inputs:  " << manager->num_vars() << std::endl
              << "[i] num_outputs: " << fs.size() << std::endl;
  }
  std::cout << format( "[i] run-time (read): %.2f secs" ) % rib_statistics->get<double>( "runtime" ) << std::endl;

  if ( level > manager->num_vars() )
  {
    std::cerr << "[e] invalid level (must be less or equal to " << manager->num_vars() << ")" << std::endl;
    return 1;
  }

  auto settings = std::make_shared<properties>();
  auto statistics = std::make_shared<properties>();

  //if ( opts.is_set( "verbose" ) ) { std::cout << "[i] start approximation" << std::endl; }
  auto fshat = ( mode == 5u ) ? fs : bdd_level_approximation( fs, (bdd_level_approximation_mode)mode, level, settings, statistics );
  //if ( opts.is_set( "verbose" ) ) { std::cout << "[i] stop approximation" << std::endl; }

  /* print? */
  if ( opts.is_set( "print" ) )
  {
    std::cout << "[i] Original function:" << std::endl;
    print_paths( fs );
    std::cout << "[i] Approximated function:" << std::endl;
    print_paths( fshat );
  }

  /* truth table? */
  auto metric_settings = std::make_shared<properties>();
  if ( opts.is_set( "truthtable" ) )
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

  /* write file? */
  if ( !outname.empty() )
  {
    rib_statistics->set( "complement_optimization", true );
    write_from_bdd( fshat, outname, rib_statistics ); /* statistics from rib contain input and output labels */
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

  return 0;
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
