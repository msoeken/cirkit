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

#include "expr.hpp"

#include <boost/program_options.hpp>

#include <classical/cli/stores.hpp>
#include <classical/utils/expression_parser.hpp>

using namespace boost::program_options;

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

expr_command::expr_command( const environment::ptr& env )
  : command( env, "Load expressions" )
{
  add_positional_option( "load" );
  opts.add_options()
    ( "load,l", value( &load ), "expression to load" )
    ( "new,n",                  "create new store entry" )
    ;
}

bool expr_command::execute()
{
  auto& exprs = env->store<expression_t::ptr>();

  if ( exprs.empty() || is_set( "new" ) )
  {
    exprs.extend();
  }

  exprs.current() = parse_expression( load );

  return true;
}

command::log_opt_t expr_command::log() const
{
  return log_opt_t({{"load", load}});
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
