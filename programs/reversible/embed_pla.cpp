/* CirKit: A circuit toolkit
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

#include <core/utils/program_options.hpp>
#include <core/utils/timeout.hpp>

#include <classical/optimization/compact_dsop.hpp>

#include <reversible/circuit.hpp>
#include <reversible/rcbdd.hpp>
#include <reversible/truth_table.hpp>
#include <reversible/functions/extend_pla.hpp>
#include <reversible/io/read_pla.hpp>
#include <reversible/io/write_pla.hpp>
#include <reversible/synthesis/embed_pla.hpp>
#include <reversible/synthesis/embed_pla_bennett.hpp>

#include <boost/format.hpp>

using namespace cirkit;

int main( int argc, char ** argv )
{
  using boost::program_options::value;

  std::string filename;
  auto        mode = 0u;
  auto        dsop = 0u;
  std::string planame;

  program_options opts;
  opts.add_options()
    ( "filename", value( &filename ),          "PLA filename" )
    ( "mode",     value_with_default( &mode ), "0: Exact cube-based embedding\n1: Heuristic BDD-based embedding" )
    ( "dsop",     value_with_default( &dsop ), "0: Naive extending\n1: Compact DSOP" )
    ( "planame",  value( &planame ),           "Filename of the embedded PLA file (default is empty)" )
    ( "truth_table,t",                         "Prints truth table of embedded PLA (with constants and garbage)" )
    ( "verbose,v",                             "Be verbose" )
    ;
  opts.parse( argc, argv );

  if ( !opts.good() || !opts.is_set( "filename" ) )
  {
    std::cout << opts << std::endl;
    return 1;
  }

  /* extend for exact embedding */
  if ( mode == 0u )
  {
    if ( opts.is_set( "verbose" ) )
    {
      std::cout << "[i] generate DSOP..." << std::endl;
    }
    if ( dsop == 0u )
    {
      binary_truth_table pla, extended;
      read_pla_settings rp_settings;
      rp_settings.extend = false;
      read_pla( pla, filename, rp_settings );
      extend_pla( pla, extended );
      write_pla( extended, "/tmp/extended.pla" );
    }
    else if ( dsop == 1u )
    {
      properties::ptr cd_settings( new properties );
      cd_settings->set( "verbose", opts.is_set( "verbose" ) );
      cd_settings->set( "sortfunc", sort_cube_meta_func_t( sort_by_dimension_first ) );
      cd_settings->set( "optfunc", opt_cube_func_t( opt_dsop_3 ) );
      compact_dsop( "/tmp/extended.pla", filename, cd_settings );
    }
  }

  if ( opts.is_set( "verbose" ) )
  {
    std::cout << "[i] embedding pla..." << std::endl;
  }

  rcbdd cf;
  properties::ptr settings( new properties );
  properties::ptr statistics( new properties );
  settings->set( "truth_table", opts.is_set( "truth_table" ) );
  settings->set( "verbose",     opts.is_set( "verbose" ) );
  if ( !planame.empty() )
  {
      settings->set( "write_pla", planame );
  }
  pla_embedding_func embedding = ( mode == 0u ) ? embed_pla_func( settings, statistics ) : embed_pla_bennett_func( settings, statistics );
  embedding( cf, mode == 0u ? "/tmp/extended.pla" : filename );

  std::cout << boost::format( "Runtime: %.2f" ) % statistics->get<double>( "runtime" ) << std::endl;

  if ( mode == 1u )
  {
    std::cout << boost::format( "Runtime (read): %.2f" ) % statistics->get<double>( "runtime_read" ) << std::endl;
  }

  return 0;
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
