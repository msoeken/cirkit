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
 * @file aig_cone.hpp
 *
 * @brief Computes a new smaller AIG based on output cones
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef AIG_CONE_HPP
#define AIG_CONE_HPP

#include <string>
#include <vector>

#include <core/properties.hpp>
#include <classical/aig.hpp>

namespace cirkit
{

/**
 * @brief Computes a new smaller AIG based on output cones
 *
 * This function takes a combinational AIG, extracts all cones based on given
 * output names and returns a new AIG that consists of only those outputs and
 * the union of all structural cones.  Names of primary inputs and primary outputs
 * are kept, but internal node ids are adjusted.
 *
 * @param aig   Source AIG
 * @param names Names of primary outputs
 * @param settings The following settings are possible
 *                 +---------+------+---------+
 *                 | Name    | Type | Default |
 *                 +---------+------+---------+
 *                 | verbose | bool | false   |
 *                 +---------+------+---------+
 * @param statistics The following statistics are given
 *                 +---------+--------+---------------------------+
 *                 | Name    | Type   | Description               |
 *                 +---------+--------+---------------------------+
 *                 | runtime | double | Runtime of the SAT solver |
 *                 +---------+--------+---------------------------+
 */
aig_graph aig_cone( const aig_graph& aig, const std::vector<std::string>& names,
                    const properties::ptr& settings = properties::ptr(),
                    const properties::ptr& statistics = properties::ptr() );

/**
 * @brief Computes a smaller AIG based on output cones
 *
 * Version that takes output indexes instead of names.
 */
aig_graph aig_cone( const aig_graph& aig, const std::vector<unsigned>& index,
                    const properties::ptr& settings = properties::ptr(),
                    const properties::ptr& statistics = properties::ptr() );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
