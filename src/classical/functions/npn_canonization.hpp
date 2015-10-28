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
 * @file npn_canonization.hpp
 *
 * @brief Computes an NPN canonization for a truth table
 *
 * @author Mathias Soeken
 * @since  2.1
 */

#ifndef NPN_CANONIZATION_HPP
#define NPN_CANONIZATION_HPP

#include <boost/dynamic_bitset.hpp>

#include <classical/utils/truth_table_utils.hpp>

namespace cirkit
{

tt exact_npn_canonization( const tt& t, boost::dynamic_bitset<>& phase, std::vector<unsigned>& perm );
tt npn_canonization( const tt& t, boost::dynamic_bitset<>& phase, std::vector<unsigned>& perm );
tt npn_canonization2( const tt& t, boost::dynamic_bitset<>& phase, std::vector<unsigned>& perm );
tt tt_from_npn( const tt& npn, const boost::dynamic_bitset<>& phase, std::vector<unsigned>& perm );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
