/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2016  EPFL
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

#include <iostream>
#include <thread>

#include <core/io/read_pla_to_bdd.hpp>
#include <core/utils/program_options.hpp>
#include <core/utils/timeout.hpp>
#include <core/utils/timer.hpp>

#include <reversible/truth_table.hpp>
#include <reversible/functions/approximate_additional_lines.hpp>
#include <reversible/functions/calculate_additional_lines.hpp>
#include <reversible/functions/extend_pla.hpp>
#include <reversible/io/read_pla.hpp>
#include <reversible/io/write_pla.hpp>

using namespace cirkit;

int main( int argc, char ** argv )
{
  using boost::program_options::value;

  std::string filename;
  unsigned    mode           = 0u;
  auto        post_compact   = true;
  auto        timeout        = 5000u;
  auto        dumpadd        = false;
  auto        explicit_zeros = false;
  std::string tmpname        = "/tmp/test.pla";
  std::string dotname;

  program_options opts;
  opts.add_options()
    ( "filename",       value( &filename ),                    "PLA filename" )
    ( "mode",           value_with_default( &mode ),           "Mode (0: extend, 1: BDD, 2: approximate)" )
    ( "post_compact",   value_with_default( &post_compact ),   "Compress PLA after extending (only for mode = 0)" )
    ( "timeout",        value_with_default( &timeout ),        "Timeout in seconds" )
    ( "tmpname",        value_with_default( &tmpname ),        "Temporary filename for extended PLA" )
    ( "dotname",        value( &dotname ),                     "If non-empty and mode = 1, the BDD is dumped to that file" )
    ( "dumpadd",        value_with_default( &dumpadd ),        "Dump ADD instead of BDD, if dotname is set and mode = 1" )
    ( "explicit_zeros", value_with_default( &explicit_zeros ), "Have explicit zeros in characteristic BDD, if mode = 1" )
    ( "verbose,v",                                             "Be verbose" )
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
  settings->set( "verbose", opts.is_set( "verbose" ) );
  properties::ptr statistics( new properties );

  /* timeout */
  std::thread t1( [&timeout]() { timeout_after( timeout ); } );

  /* stop time */
  print_timer t;

  if ( mode == 0u )
  {
    binary_truth_table pla, extended;
    read_pla_settings rp_settings;
    rp_settings.extend = false;
    read_pla( pla, filename, rp_settings );
    extend_pla_settings ep_settings;
    ep_settings.post_compact = post_compact;
    ep_settings.verbose = opts.is_set( "verbose" );
    extend_pla( pla, extended, ep_settings );

    write_pla( extended, tmpname );

    additional = approximate_additional_lines( tmpname, settings, statistics );
  }
  else if ( mode == 1u )
  {
    settings->set( "dotname",        dotname );
    settings->set( "dumpadd",        dumpadd );
    settings->set( "explicit_zeros", explicit_zeros );
    additional = calculate_additional_lines( filename, settings, statistics );
  }
  else if ( mode == 2u )
  {
    additional = approximate_additional_lines( filename, settings, statistics );
  }

  std::cout << "Inputs:     " << statistics->get<unsigned>( "num_inputs" ) << std::endl
            << "Outputs:    " << statistics->get<unsigned>( "num_outputs" ) << std::endl
            << "Additional: " << additional << std::endl;

  t1.detach();

  return 0;
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
