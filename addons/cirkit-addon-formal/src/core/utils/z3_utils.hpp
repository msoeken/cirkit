/* RevKit: A Toolkit for Reversible Circuit Design (www.revkit.org)
 * Copyright (C) 2009-2011  The RevKit Developers <revkit@informatik.uni-bremen.de>
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
 */

#ifndef Z3_UTILS_HPP
#define Z3_UTILS_HPP

#include <z3++.h>

#include <boost/dynamic_bitset.hpp>

namespace cirkit
{

z3::expr operator<<(const z3::expr& a, const z3::expr& b);
z3::expr ite(const z3::expr& a, const z3::expr& b, const z3::expr& c);
boost::dynamic_bitset<> to_bitset( const z3::expr& a );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
