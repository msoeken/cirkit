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

#include "write_aiger.hpp"

#include <iostream>

#include <core/utils/program_options.hpp>
#include <classical/io/write_aiger.hpp>

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

write_aiger_command::write_aiger_command( const environment::ptr& env ) : aig_base_command( env, "Writes AIG to file (in AIGER format)" )
{
  add_positional_option( "filename" );
  opts.add_options()
    ( "filename", value( &filename ), "AIGER filename" )
    ;
}

bool write_aiger_command::execute()
{
  if ( is_verbose() )
  {
    std::cout << "[i] write to " << filename << std::endl;
  }
  write_aiger( aig(), filename );
  return true;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
