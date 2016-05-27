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
 * @file aig_npn_canonization.hpp
 *
 * @brief NPN canonization using AIGs
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef AIG_NPN_CANONIZATION_HPP
#define AIG_NPN_CANONIZATION_HPP

#include <vector>

#include <boost/dynamic_bitset.hpp>

#include <core/properties.hpp>
#include <classical/aig.hpp>

namespace cirkit
{

/* requires a single-output AIG */
void aig_npn_canonization_flip_swap( const aig_graph& aig, boost::dynamic_bitset<>& phase, std::vector<unsigned>& perm,
                                     const properties::ptr& settings = properties::ptr(),
                                     const properties::ptr& statistics = properties::ptr() );

/* requires a single-output AIG */
void aig_npn_canonization_flip_swap_shared_miter( const aig_graph& aig, boost::dynamic_bitset<>& phase, std::vector<unsigned>& perm,
                                                  const properties::ptr& settings = properties::ptr(),
                                                  const properties::ptr& statistics = properties::ptr() );

/* requires a single-output AIG */
void aig_npn_canonization_sifting( const aig_graph& aig, boost::dynamic_bitset<>& phase, std::vector<unsigned>& perm,
                                   const properties::ptr& settings = properties::ptr(),
                                   const properties::ptr& statistics = properties::ptr() );

/* requires a single-output AIG */
void aig_npn_canonization_sifting_shared_miter( const aig_graph& aig, boost::dynamic_bitset<>& phase, std::vector<unsigned>& perm,
                                                const properties::ptr& settings = properties::ptr(),
                                                const properties::ptr& statistics = properties::ptr() );

/* canonicizes all outputs and builds a new AIG */
aig_graph aig_npn_canonization( const aig_graph& aig,
                                const properties::ptr& settings = properties::ptr(),
                                const properties::ptr& statistics = properties::ptr() );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
