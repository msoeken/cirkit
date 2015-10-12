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

#include "read_symmetries.hpp"

#include <boost/algorithm/string/trim.hpp>

#include <core/utils/string_utils.hpp>
#include <classical/utils/aig_utils.hpp>

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

void read_symmetries( aig_graph& aig, const std::string& filename )
{
  auto& info = aig_info( aig );
  auto& symmetries = info.input_symmetries;

  foreach_line_in_file( filename, [&]( const std::string& line ) {
      auto p = split_string_pair( line, "=" );
      boost::trim( p.first );
      boost::trim( p.second );

      auto n1 = aig_node_by_name( info, p.first );
      auto n2 = aig_node_by_name( info, p.second );

      symmetries.push_back( {n1,n2} );
    } );
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
