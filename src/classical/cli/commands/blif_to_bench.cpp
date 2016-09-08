/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2016  EPFL
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
