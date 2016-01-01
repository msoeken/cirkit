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

#include "read_sym.hpp"

#include <boost/algorithm/string/trim.hpp>

#include <core/cli/rules.hpp>
#include <core/utils/range_utils.hpp>
#include <core/utils/string_utils.hpp>
#include <classical/io/read_symmetries.hpp>
#include <classical/utils/aig_utils.hpp>

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

read_sym_command::read_sym_command( const environment::ptr& env )
  : aig_base_command( env, "Read input symmetries" )
{
  opts.add_options()
    ( "filename", value( &filename ), "Input symmetries filename" )
    ;
}

command::rules_t read_sym_command::validity_rules() const
{
  return {file_exists( filename, "filename" )};
}

bool read_sym_command::execute()
{
  read_symmetries( aig(), filename );
  return true;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
