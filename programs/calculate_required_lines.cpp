/* RevKit: A Toolkit for Reversible Circuit Design (www.revkit.org)
 * Copyright (C) 2009-2014  The RevKit Developers <revkit@informatik.uni-bremen.de>
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

#include <iostream>
#include <thread>

#include <core/io/read_pla_to_bdd.hpp>
#include <core/utils/timeout.hpp>
#include <core/utils/timer.hpp>

#include <reversible/truth_table.hpp>
#include <reversible/functions/approximate_additional_lines.hpp>
#include <reversible/functions/calculate_additional_lines.hpp>
#include <reversible/functions/extend_pla.hpp>
#include <reversible/io/read_pla.hpp>
#include <reversible/io/write_pla.hpp>
#include <reversible/utils/program_options.hpp>

using namespace cirkit;

int main( int argc, char ** argv )
{
  using boost::program_options::value;

  std::string filename;
  unsigned    mode    = 0u;
  unsigned    timeout = 5000u;
  bool        verbose = false;

  program_options opts;
  opts.add_options()
    ( "filename", value<std::string>( &filename ),                           "PLA filename" )
    ( "mode",     value<unsigned>   ( &mode     )->default_value( mode    ), "Mode (0: extend, 1: BDD)" )
    ( "timeout",  value<unsigned>   ( &timeout  )->default_value( timeout ), "Timeout in seconds" )
    ( "verbose",  value<bool>       ( &verbose  )->default_value( verbose ), "Be verbose" )
    ;

  opts.parse( argc, argv );

  if ( !opts.good() || !opts.is_set( "filename" ) )
  {
    std::cout << opts << std::endl;
    return 1;
  }

  /* settings and statistics */
  unsigned additional = 0u;
  properties::ptr settings( new properties );
  settings->set( "verbose", verbose );
  properties::ptr statistics( new properties );

  /* timeout */
  std::thread t1( [&timeout]() { timeout_after( timeout ); } );

  /* stop time */
  print_timer pt( std::cout );
  timer<print_timer> timer( pt );

  if ( mode == 0u )
  {
    binary_truth_table pla, extended;
    read_pla_settings rp_settings;
    rp_settings.extend = false;
    read_pla( pla, filename, rp_settings );
    extend_pla( pla, extended );

    write_pla( extended, "/tmp/test.pla" );

    additional = approximate_additional_lines( "/tmp/test.pla", settings, statistics );
  }
  else if ( mode == 1u )
  {
    additional = calculate_additional_lines( filename, settings, statistics );
  }

  std::cout << "Inputs:     " << statistics->get<unsigned>( "num_inputs" ) << std::endl
            << "Outputs:    " << statistics->get<unsigned>( "num_outputs" ) << std::endl
            << "Additional: " << additional << std::endl;

  t1.detach();

  return 0;
}

// Local Variables:
// c-basic-offset: 2
// End:
