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
 * @file z3_utils.hpp
 *
 * @brief Helper functions to extend the Z3 C++ API.
 *
 * @author Mathias Soeken
 * @author Heinz Riener
 * @since 2.0
 */

#ifndef Z3_UTILS_HPP
#define Z3_UTILS_HPP

#include <boost/dynamic_bitset.hpp>

#include <z3++.h>

namespace cirkit
{

void check_error( const z3::context& ctx );
z3::expr operator<<(const z3::expr& a, const z3::expr& b);
z3::expr logic_xor( const z3::expr& a, const z3::expr& b );
boost::dynamic_bitset<> to_bitset( const z3::expr& a );
const bool expr_to_bool( const z3::expr& e );
const std::string expr_to_bin( const z3::expr &e );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
