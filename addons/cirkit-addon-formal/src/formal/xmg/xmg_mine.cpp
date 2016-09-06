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

#include "xmg_mine.hpp"

#include <fstream>
#include <iostream>

#include <boost/algorithm/string/trim.hpp>
#include <boost/optional.hpp>

#include <core/utils/string_utils.hpp>
#include <classical/utils/truth_table_utils.hpp>
#include <formal/xmg/xmg_minlib.hpp>

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

void xmg_mine( const std::string& lut_file, const std::string& opt_file, const properties::ptr& settings, const properties::ptr& statistics )
{
  xmg_minlib_manager minlib( settings );
  minlib.load_library_string( xmg_minlib_manager::npn2_s );
  minlib.load_library_string( xmg_minlib_manager::npn3_s );
  minlib.load_library_string( xmg_minlib_manager::npn4_s );

  minlib.load_library_file( opt_file, true );

  std::ifstream in( lut_file.c_str(), std::ifstream::in );
  std::string line;

  while ( getline( in, line ) )
  {
    boost::trim( line );

    tt t( line );

    minlib.find_xmg_no_npn( t );
  }
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
