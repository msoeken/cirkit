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

#include "blif_to_bench.hpp"

#include <boost/program_options.hpp>

#include <alice/rules.hpp>

#include <classical/io/read_blif.hpp>
#include <classical/io/write_bench.hpp>

using boost::program_options::value;

namespace cirkit
{

blif_to_bench_command::blif_to_bench_command( const environment::ptr& env )
  : cirkit_command( env, "BLIF to BENCH converter" )
{
  add_positional_option( "blif_name" );
  add_positional_option( "bench_name" );
  opts.add_options()
    ( "blif_name",  value( &blif_name ),  "filename for BLIF file" )
    ( "bench_name", value( &bench_name ), "filename for BENCH file" )
    ;
}

command::rules_t blif_to_bench_command::validity_rules() const
{
  return {
    {[this]() { return is_set( "blif_name" ) && is_set( "bench_name" ); }, "one must specify filenames for BLIF file and BENCH file"},
    file_exists( blif_name, "blif_name" )
  };
}

bool blif_to_bench_command::execute()
{
  auto g = read_blif( blif_name );
  write_bench( g, bench_name );

  return true;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
