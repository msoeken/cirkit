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

#include "worstcase.hpp"

#include <core/utils/program_options.hpp>
#include <classical/cli/stores.hpp>
#include <classical/approximate/worst_case.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

worstcase_command::worstcase_command( const environment::ptr& env )
  : cirkit_command( env, "Computes worst-case error for AIGs" )
{
  opts.add_options()
    ( "id1", value_with_default( &id1 ), "id of first circuit" )
    ( "id2", value_with_default( &id2 ), "id of second circuit" )
    ;
  be_verbose();
}

bool worstcase_command::execute()
{
  const auto& aigs = env->store<aig_graph>();

  auto settings = make_settings();

  std::cout << worst_case( aigs[id1], aigs[id2], settings, statistics ) << std::endl;

  print_runtime();

  return true;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
