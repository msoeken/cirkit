/* RevKit (www.revkit.org)
 * Copyright (C) 2009-2015  University of Bremen
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

/**
 * @author Mathias Soeken
 */

#include <thread>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/min.hpp>
#include <boost/format.hpp>
#include <boost/range/algorithm.hpp>

#include <core/utils/range_utils.hpp>
#include <core/utils/timeout.hpp>

#include <classical/optimization/esop_minimization.hpp>
#include <classical/optimization/exorcism_minimization.hpp>

#include <reversible/circuit.hpp>
#include <reversible/rcbdd.hpp>
#include <reversible/truth_table.hpp>
#include <reversible/functions/extend_pla.hpp>
#include <reversible/io/read_pla.hpp>
#include <reversible/io/print_statistics.hpp>
#include <reversible/io/write_pla.hpp>
#include <reversible/io/write_realization.hpp>
#include <reversible/utils/reversible_program_options.hpp>
#include <reversible/synthesis/embed_pla.hpp>
#include <reversible/synthesis/rcbdd_synthesis.hpp>

using namespace cirkit;

std::tuple<int, int, double> compute_stats( const std::vector<int>& sizes )
{
  using namespace boost::accumulators;

  accumulator_set<double, stats<tag::mean, tag::min, tag::max>> acc;
  acc = boost::for_each( sizes, acc );

  return std::make_tuple( min( acc ), max( acc ), mean( acc ) );
}

int main( int argc, char ** argv )
{
  using boost::program_options::value;

  std::string filename;
  auto        mode           = 0u;
  auto        method         = 0u;
  auto        smart_pickcube = true;
  std::string embedded_pla;
  auto        esop_minimizer = 0u;

  reversible_program_options opts;
  opts.add_write_realization_option();
  opts.add_options()
    ( "filename",       value( &filename ),                    "PLA filename" )
    ( "mode",           value_with_default( &mode ),           "Mode (0: default, 1: swap, 2: hamming)" )
    //( "method",         value_with_default( &method ),         "Method (0: resolve cycles, 1: transpositions from front, 2: transpositions from back)" )
    //( "smart_pickcube", value_with_default( &smart_pickcube ), "Use smarter version of pickcube" )
    ( "embedded_pla",   value( &embedded_pla ),                "Filename of the embedded PLA file (default is empty)" )
    ( "truth_table,t",                                         "Prints truth table of embedded PLA (with constants and garbage)" )
    ( "esop_minimizer", value_with_default( &esop_minimizer ), "ESOP minizer (0: built-in, 1: exorcism)" )
    ( "verbose,v",                                             "Be verbose" )
    ;
  opts.parse( argc, argv );

  if ( !opts.good() || !opts.is_set( "filename" ) )
  {
    std::cout << opts << std::endl;
    return 1;
  }

  binary_truth_table pla, extended;
  rcbdd cf;
  circuit circ;

  read_pla_settings settings;
  settings.extend = false;
  if ( opts.is_set( "verbose" ) ) { std::cout << "[i] read PLA" << std::endl; }
  read_pla( pla, filename, settings );
  if ( opts.is_set( "verbose" ) ) { std::cout << "[i] extend PLA" << std::endl; }
  extend_pla( pla, extended );
  write_pla( extended, "/tmp/extended.pla" );

  auto ep_settings = std::make_shared<properties>();
  ep_settings->set( "truth_table", opts.is_set( "truth_table" ) );
  ep_settings->set( "write_pla", embedded_pla );
  if ( opts.is_set( "verbose" ) ) { std::cout << "[i] embed PLA" << std::endl; }
  embed_pla( cf, "/tmp/extended.pla", ep_settings );

  auto rs_settings = std::make_shared<properties>();
  auto rs_statistics = std::make_shared<properties>();
  rs_settings->set( "verbose", opts.is_set( "verbose" ) );
  rs_settings->set( "mode", mode );
  rs_settings->set( "synthesis_method", (SynthesisMethod)method );
  rs_settings->set( "smart_pickcube", smart_pickcube );
  auto esopmin_settings = std::make_shared<properties>();
  esopmin_settings->set( "verbose", opts.is_set( "verbose" ) );
  rs_settings->set( "esopmin", esop_minimizer ? dd_based_exorcism_minimization_func( esopmin_settings ) : dd_based_esop_minimization_func( esopmin_settings ) );
  rcbdd_synthesis( circ, cf, rs_settings, rs_statistics );

  if ( opts.is_write_realization_filename_set() )
  {
    write_realization( circ, opts.write_realization_filename() );
  }

  print_statistics_settings ps_settings;
  auto stats = compute_stats( rs_statistics->get<std::vector<int>>( "node_count" ) );
  ps_settings.main_template += boost::str( boost::format( "Sizes:        (%d, %d, %.2f)\nAccess:       %d\n" )
                                           % std::get<0>( stats ) % std::get<1>( stats ) % std::get<2>( stats )
                                           % rs_statistics->get<unsigned long long>( "access" ) );
  print_statistics( circ, rs_statistics->get<double>( "runtime" ), ps_settings );

  return 0;
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
