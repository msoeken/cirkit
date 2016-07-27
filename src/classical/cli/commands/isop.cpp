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
