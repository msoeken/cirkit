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
