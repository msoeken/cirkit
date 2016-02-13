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
 * @file compute_levels.hpp
 *
 * @brief Compute levels of the AIG
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef COMPUTE_LEVELS_HPP
#define COMPUTE_LEVELS_HPP

#include <map>
#include <vector>

#include <core/properties.hpp>
#include <classical/aig.hpp>

namespace cirkit
{

/**
 * @brief Maps each aig_node to a level
 */
std::map<aig_node, unsigned> compute_levels( const aig_graph& aig,
                                             const properties::ptr& settings = properties::ptr(),
                                             const properties::ptr& statistics = properties::ptr() );

/**
 * @brief Maps each level to a vector of aig_nodes
 *
 * Internally calls compute_levels
 */
std::vector<std::vector<aig_node>> levelize_nodes( const aig_graph& aig,
                                                   const properties::ptr& settings = properties::ptr(),
                                                   const properties::ptr& statistics = properties::ptr() );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
