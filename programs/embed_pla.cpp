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

#include <reversible/circuit.hpp>
#include <reversible/rcbdd.hpp>
#include <reversible/truth_table.hpp>
#include <reversible/functions/extend_pla.hpp>
#include <reversible/io/read_pla.hpp>
#include <reversible/io/write_pla.hpp>
#include <reversible/utils/program_options.hpp>
#include <reversible/synthesis/embed_pla.hpp>

#include <boost/format.hpp>

using namespace cirkit;

int main( int argc, char ** argv )
{
  using boost::program_options::value;

  std::string filename;
  std::string planame;

  program_options opts;
  opts.add_options()
    ( "filename", value<std::string>( &filename ), "PLA filename" )
    ( "planame",  value<std::string>( &planame ),  "Filename of the embedded PLA file (default is empty)" )
    ( "truth_table,t",                             "Prints truth table of embedded PLA (with constants and garbage)" )
    ( "verbose,v",                                 "Be verbose" )
    ;
  opts.parse( argc, argv );

  if ( !opts.good() || !opts.is_set( "filename" ) )
  {
    std::cout << opts << std::endl;
    return 1;
  }

  binary_truth_table pla, extended;
  rcbdd cf;

  read_pla_settings rp_settings;
  rp_settings.extend = false;
  read_pla( pla, filename, rp_settings );
  extend_pla( pla, extended );
  write_pla( extended, "/tmp/extended.pla" );

  properties::ptr settings( new properties );
  properties::ptr statistics( new properties );
  settings->set( "truth_table", opts.is_set( "truth_table" ) );
  settings->set( "verbose",     opts.is_set( "verbose" ) );
  if ( !planame.empty() )
  {
      settings->set( "write_pla", planame );
  }
  embed_pla( cf, "/tmp/extended.pla", settings, statistics );

  std::cout << boost::format( "Runtime: %.2f" ) % statistics->get<double>( "runtime" ) << std::endl;

  return 0;
}

// Local Variables:
// c-basic-offset: 2
// End:

