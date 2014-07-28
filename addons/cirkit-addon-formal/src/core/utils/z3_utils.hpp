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
 * @file exact_synthesis.hpp
 *
 * @brief Exact Synthesis of Reversible Networks
 */

#ifndef Z3_UTILS_HPP
#define Z3_UTILS_HPP

#include <boost/dynamic_bitset.hpp>

#include <z3++.h>

namespace cirkit
{

z3::expr operator<<(z3::expr const & a, z3::expr const & b);
z3::expr ite(z3::expr const & a, z3::expr const & b, z3::expr const & c);
boost::dynamic_bitset<> to_bitset( const z3::expr& a );

}

#endif

// Local Variables:
// c-basic-offset: 2
// End:
