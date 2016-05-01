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

#include "output_noise.hpp"

#include <core/utils/program_options.hpp>
#include <classical/functions/add_output_noise.hpp>

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

output_noise_command::output_noise_command( const environment::ptr& env ) : aig_base_command( env, "Adds random noise to the outputs of the circuit" )
{
  opts.add_options()
    ( "levels,l", value_with_default( &levels ), "Number of levels" )
    ( "keep,k",                                  "Keep old outputs" )
    ( "seed",     value( &seed ),                "Random seed (current time, unless set)" )
    ;
  be_verbose();
}

bool output_noise_command::execute()
{
  auto settings = make_settings();
  settings->set( "num_levels", levels );
  settings->set( "keep_outputs", is_set( "keep" ) );
  if ( is_set( "seed" ) )
  {
    settings->set( "seed", seed );
  }
  aig() = add_output_noise( aig(), settings );
  return true;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
