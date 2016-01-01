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

#include "rename.hpp"

#include <map>

#include <core/utils/program_options.hpp>
#include <core/utils/string_utils.hpp>
#include <classical/functions/aig_rename.hpp>

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

rename_command::rename_command( const environment::ptr& env )
  : aig_base_command( env, "Renames inputs and outputs of AIG" )
{
  opts.add_options()
    ( "input,i",   value( &inputs )->composing(),  "Rename inputs, oldname=newname" )
    ( "output,o",  value( &outputs )->composing(), "Rename outputs, oldname=newname" )
    ;
}

bool rename_command::execute()
{
  std::map<std::string, std::string> imap, omap;
  for ( const auto& input : inputs )
  {
    imap.insert( split_string_pair( input, "=" ) );
  }
  for ( const auto& output : outputs )
  {
    omap.insert( split_string_pair( output, "=" ) );
  }

  aig_rename( aig(), imap, omap );

  return true;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
