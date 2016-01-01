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

#include "propagate.hpp"

#include <map>

#include <boost/format.hpp>

#include <core/utils/program_options.hpp>
#include <core/utils/string_utils.hpp>
#include <classical/functions/aig_constant_propagation.hpp>

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

propagate_command::propagate_command( const environment::ptr& env )
  : aig_base_command( env, "Propagates constant inputs in an AIG" )
{
  opts.add_options()
    ( "assignments,a", value( &assignments ), "Propagate assignment (e.g. \"!a,b,c\")" )
    ;
  be_verbose();
}

bool propagate_command::execute()
{
  auto settings = make_settings();
  auto statistics = std::make_shared<properties>();
  std::map<std::string, bool> propagation_values;
  foreach_string( assignments, ",", [&]( const std::string& s ) {
      auto name = ( s[0] == '!' ) ? s.substr( 1u ) : s;
      auto value = ( s[0] != '!' );
      if ( is_verbose() )
      {
        std::cout << "[i] assign " << name << " <- " << value << std::endl;
      }
      propagation_values.insert( {name, value} );
    });
  aig() = aig_constant_propagation( aig(), propagation_values, settings, statistics );
  std::cout << boost::format( "[i] Run-time: %.2f secs" ) % statistics->get<double>( "runtime" ) << std::endl;
  return true;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
