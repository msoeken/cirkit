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

#include "support.hpp"

#include <iostream>

#include <boost/format.hpp>

#include <core/utils/bitset_utils.hpp>
#include <classical/functions/aig_support.hpp>

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

support_command::support_command( const environment::ptr& env ) : aig_base_command( env, "Computes the structural support of the AIG" )
{
  be_verbose();
}

bool support_command::execute()
{
  auto settings = make_settings();
  auto statistics = std::make_shared<properties>();

  auto support = aig_structural_support( aig(), settings, statistics );

  std::cout << "[i] structural support" << std::endl;

  const auto& _info = info();
  for ( const auto& output : _info.outputs )
  {
    std::cout << boost::format( "    %s :" ) % output.second;
    foreach_bit( support.at( output.first ), [&]( unsigned pos ) {
        std::cout << " " << _info.node_names.at( _info.inputs.at( pos ) );
      } );
    std::cout << std::endl;
  }
  std::cout << boost::format( "[i] Run-time: %.2f secs" ) % statistics->get<double>( "runtime" ) << std::endl;

  return true;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
