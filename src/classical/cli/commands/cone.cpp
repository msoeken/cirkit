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

#include "cone.hpp"

#include <boost/format.hpp>

#include <core/utils/program_options.hpp>
#include <classical/functions/aig_cone.hpp>

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

cone_command::cone_command( const environment::ptr& env ) : aig_base_command( env, "Extracts cone by outputs" )
{
  opts.add_options()
    ( "output,o", value( &outputs )->composing(), "Names of outputs that should be kept" )
    ;
  be_verbose();
}

command::rules_t cone_command::validity_rules() const
{
  return {
    {[&]() { return !outputs.empty(); }, "no output name specified" },
  };
}

bool cone_command::execute()
{
  auto settings = make_settings();
  auto statistics = std::make_shared<properties>();
  aig() = aig_cone( aig(), outputs, settings, statistics );
  std::cout << boost::format( "[i] Run-time: %.2f secs" ) % statistics->get<double>( "runtime" ) << std::endl;

  return true;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
