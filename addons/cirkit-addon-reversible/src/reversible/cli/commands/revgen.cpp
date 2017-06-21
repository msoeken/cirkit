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

#include "revgen.hpp"

#include <iostream>

#include <boost/dynamic_bitset.hpp>
#include <boost/program_options.hpp>

#include <core/utils/bitset_utils.hpp>
#include <reversible/truth_table.hpp>
#include <reversible/cli/stores.hpp>

namespace cirkit
{

using boost::program_options::value;

revgen_command::revgen_command( const environment::ptr& env )
  : cirkit_command( env, "Generate reversible structures" )
{
  opts.add_options()
    ( "hwb", value( &hwb ), "Generate reversible HWB function" )
    ;
}

bool revgen_command::execute()
{
  if ( is_set( "hwb" ) )
  {
    binary_truth_table spec;

    boost::dynamic_bitset<> b( hwb );

    spec.add_entry( number_to_truth_table_cube( 0u, hwb ), number_to_truth_table_cube( 0u, hwb ) );

    do {
      if ( b.any() )
      {
        const auto shift = b.count();
        const auto out = ( b << shift ) | ( b >> ( hwb - shift ) );

        const auto c_in = number_to_truth_table_cube( b.to_ulong(), hwb );
        const auto c_out = number_to_truth_table_cube( out.to_ulong(), hwb );

        spec.add_entry( c_in, c_out );
      }

      inc( b );
    } while ( b.any() );

    auto& specs = env->store<binary_truth_table>();

    if ( specs.empty() )
    {
      specs.extend();
    }
    specs.current() = spec;
  }

  return true;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
