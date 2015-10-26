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

#include "feather.hpp"

#include <core/utils/program_options.hpp>
#include <classical/functions/feathering.hpp>

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

feather_command::feather_command( const environment::ptr& env ) : aig_base_command( env, "Adds outputs to higher levels of a circuit" )
{
  opts.add_options()
    ( "levels,l",        value_with_default( &levels ),        "Number of levels to feather" )
    ( "respect_edges,r", value_with_default( &respect_edges ), "If true, then for each node within the feathering level, "
                                                               "an output is added according to the polarities of outgoing"
                                                               "edges.  Otherwise, always two outputs are created for each"
                                                               "node, if not existing." )
    ( "output_name,o",   value_with_default( &output_name ),   "Template for name of the new outputs" )
    ;
  be_verbose();
}

bool feather_command::execute()
{
  auto settings = make_settings();
  settings->set( "respect_edges", respect_edges );
  settings->set( "output_name",   output_name );
  aig() = output_feathering( aig(), levels, settings );
  return true;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
