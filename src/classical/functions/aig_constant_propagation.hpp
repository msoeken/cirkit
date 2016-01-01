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
 * @file aig_constant_propagation.hpp
 *
 * @brief Assigns and propagates constants in an AIG.
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef AIG_CONSTANT_PROPAGATION_HPP
#define AIG_CONSTANT_PROPAGATION_HPP

#include <map>

#include <core/properties.hpp>
#include <classical/aig.hpp>

namespace cirkit
{

/**
 * @brief Assigns and propagates constants in an AIG.
 *
 * @param aig AIG graph
 * @param values A map from AIG input nodes to Boolean values
 */
aig_graph aig_constant_propagation( const aig_graph& aig, const std::map<std::string, bool>& values,
                                    const properties::ptr& settings = properties::ptr(),
                                    const properties::ptr& statistics = properties::ptr() );

aig_graph aig_constant_propagation( const aig_graph& aig, const std::map<unsigned, bool>& values,
                                    const properties::ptr& settings = properties::ptr(),
                                    const properties::ptr& statistics = properties::ptr() );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
