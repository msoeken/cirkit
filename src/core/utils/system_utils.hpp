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

/**
 * @file system_utils.hpp
 *
 * @brief Some helper functions for system managing
 *
 * @author Mathias Soeken
 * @since  2.1
 */

#ifndef SYSTEM_UTILS_HPP
#define SYSTEM_UTILS_HPP

#include <string>
#include <vector>

namespace cirkit
{

using result_t = std::pair<int, std::vector<std::string>>;
result_t execute_and_return( const std::string& cmd );
result_t execute_and_return_tee( const std::string& cmd );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
