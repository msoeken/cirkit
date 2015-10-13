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
 * @file aig_support.hpp
 *
 * @brief Calculates the support of an AIG
 *
 * @author Mathias Soeken
 * @since  2.0
 */

#ifndef AIG_SUPPORT_HPP
#define AIG_SUPPORT_HPP

#include <map>

#include <boost/dynamic_bitset.hpp>

#include <core/properties.hpp>
#include <classical/aig.hpp>

namespace cirkit
{

using support_map_t = std::map<aig_function, boost::dynamic_bitset<>>;

support_map_t aig_structural_support( const aig_graph& aig,
                                      properties::ptr settings = properties::ptr(),
                                      properties::ptr statistics = properties::ptr() );

boost::dynamic_bitset<> get_functional_support( const boost::dynamic_bitset<>& u, unsigned po, unsigned num_pis );
boost::dynamic_bitset<> get_functional_support( const boost::dynamic_bitset<>& u, unsigned po, const aig_graph_info& info );
boost::dynamic_bitset<> get_functional_support( const boost::dynamic_bitset<>& u, unsigned po, const aig_graph& aig );

support_map_t aig_functional_support( const aig_graph& aig );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
