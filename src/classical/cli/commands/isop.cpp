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

#include "isop.hpp"

#include <iostream>
#include <vector>

#include <boost/dynamic_bitset.hpp>

#include <core/cube.hpp>
#include <core/utils/range_utils.hpp>
#include <classical/cli/stores.hpp>
#include <classical/functions/isop.hpp>
#include <classical/utils/truth_table_utils.hpp>

namespace cirkit
{

isop_command::isop_command( const environment::ptr& env )
  : cirkit_command( env, "Compute ISOP" )
{
  opts.add_options()
    ( "tt,t", "computes ISOP from truth table" )
    ;
}

command::rules_t isop_command::validity_rules() const
{
  return {
    {[this]() { return !is_set( "tt" ) || env->store<tt>().current_index() != -1; }, "no truth table in store" }
  };
}

bool isop_command::execute()
{
  if ( is_set( "tt" ) )
  {
    const auto& tts = env->store<tt>();

    std::vector<int> cover;
    tt_isop( tts.current(), tts.current(), cover );
    const auto sop = cover_to_cubes( cover, tt_num_vars( tts.current() ) );
    common_pla_print( sop );
  }

  return true;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
