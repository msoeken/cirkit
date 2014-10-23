/* RevKit (www.revkit.org)
 * Copyright (C) 2009-2014  University of Bremen
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

#include <thread>

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

int main( int argc, char ** argv )
{
  using boost::program_options::value;

  std::string filename;
  unsigned    mode           = 0u;
  unsigned    method         = 0u;
  bool        smart_pickcube = true;
  std::string embedded_pla;
  unsigned    timeout        = 5000u;
  unsigned    esop_minimizer = 0u;

  reversible_program_options opts;
  opts.add_write_realization_option();
  opts.add_options()
    ( "filename",       value( &filename ),                    "PLA filename" )
    ( "mode",           value_with_default( &mode ),           "Mode (0: default, 1: swap, 2: hamming)" )
    ( "method",         value_with_default( &method ),         "Method (0: resolve cycles, 1: transpositions from front, 2: transpositions from back)" )
    ( "smart_pickcube", value_with_default( &smart_pickcube ), "Use smarter version of pickcube" )
    ( "embedded_pla",   value( &embedded_pla ),                "Filename of the embedded PLA file (default is empty)" )
    ( "truth_table,t",                                         "Prints truth table of embedded PLA (with constants and garbage)" )
    //    ( "timeout",        value_with_default( &timeout ),        "Timeout in seconds" )
    ( "esop_minimizer", value_with_default( &esop_minimizer ), "ESOP minizer (0: built-in, 1: exorcism)" )
    ( "verbose,v",                                             "Be verbose" )
    ;
  opts.parse( argc, argv );

  if ( !opts.good() || !opts.is_set( "filename" ) )
  {
    std::cout << opts << std::endl;
    return 1;
  }

  /* timeout */
  //std::thread t1( [&timeout]() { timeout_after( timeout ); } );

  binary_truth_table pla, extended;
  rcbdd cf;
  circuit circ;

  read_pla_settings settings;
  settings.extend = false;
  read_pla( pla, filename, settings );
  extend_pla( pla, extended );
  write_pla( extended, "/tmp/extended.pla" );

  properties::ptr ep_settings( new properties );
  ep_settings->set( "truth_table", opts.is_set( "truth_table" ) );
  ep_settings->set( "write_pla", embedded_pla );
  embed_pla( cf, "/tmp/extended.pla", ep_settings );

  properties::ptr rs_settings( new properties );
  properties::ptr rs_statistics( new properties );
  rs_settings->set( "verbose", opts.is_set( "verbose" ) );
  rs_settings->set( "mode", mode );
  rs_settings->set( "synthesis_method", (SynthesisMethod)method );
  rs_settings->set( "smart_pickcube", smart_pickcube );
  properties::ptr esopmin_settings( new properties );
  esopmin_settings->set( "verbose", opts.is_set( "verbose" ) );
  rs_settings->set( "esopmin", esop_minimizer ? dd_based_exorcism_minimization_func( esopmin_settings ) : dd_based_esop_minimization_func( esopmin_settings ) );
  rcbdd_synthesis( circ, cf, rs_settings, rs_statistics );

  if ( opts.is_write_realization_filename_set() )
  {
    write_realization( circ, opts.write_realization_filename() );
  }

  print_statistics( circ, rs_statistics->get<double>( "runtime" ) );

  return 0;
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
