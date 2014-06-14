/* RevKit: A Toolkit for Reversible Circuit Design (www.revkit.org)
 * Copyright (C) 2009-2013  The RevKit Developers <revkit@informatik.uni-bremen.de>
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

#include <reversible/circuit.hpp>
#include <reversible/rcbdd.hpp>
#include <reversible/truth_table.hpp>
#include <reversible/functions/extend_pla.hpp>
#include <reversible/io/read_pla.hpp>
#include <reversible/io/write_pla.hpp>
#include <reversible/io/write_realization.hpp>
#include <reversible/utils/program_options.hpp>
#include <reversible/synthesis/embed_pla.hpp>
#include <reversible/synthesis/rcbdd_synthesis.hpp>

using namespace cirkit;

int main( int argc, char ** argv )
{
  using boost::program_options::value;

  std::string filename;
  std::string embedded_pla;
  bool        truth_table = false;
  bool        verbose = false;

  program_options opts;
  opts.add_write_realization_option();
  opts.add_options()
    ( "filename",     value<std::string>( &filename ),                            "PLA filename" )
    ( "embedded_pla", value<std::string>( &embedded_pla ),                        "Filename of the embedded PLA file (default is empty)" )
    ( "truth_table",  value<bool>       ( &truth_table )->default_value( false ), "Prints truth table of embedded PLA (with constants and garbage)" )
    ( "verbose",      value<bool>       ( &verbose )->default_value( false ),     "Verbose" )
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
  read_pla( pla, filename, settings );
  extend_pla( pla, extended );
  write_pla( extended, "/tmp/extended.pla" );

  properties::ptr ep_settings( new properties );
  ep_settings->set( "truth_table", truth_table );
  ep_settings->set( "write_pla", embedded_pla );
  embed_pla( cf, "/tmp/extended.pla", ep_settings );

  properties::ptr rs_settings( new properties );
  rs_settings->set( "verbose", verbose );
  rcbdd_synthesis( circ, cf, rs_settings );

  if ( opts.is_write_realization_filename_set() )
  {
    write_realization( circ, opts.write_realization_filename() );
  }

  return 0;
}

// Local Variables:
// c-basic-offset: 2
// End:

