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

/**
 * @file zdd_to_sets.hpp
 *
 * @brief Get set of sets represented by ZDD node
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef ZDD_PRETTY_PRINT_HPP
#define ZDD_PRETTY_PRINT_HPP

#include <vector>

#include <boost/dynamic_bitset.hpp>

#include <classical/dd/zdd.hpp>

namespace cirkit
{

std::vector<boost::dynamic_bitset<>> zdd_to_sets( const zdd& z );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
