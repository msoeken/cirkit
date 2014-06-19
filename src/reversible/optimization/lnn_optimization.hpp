/* RevKit: A Toolkit for Reversible Circuit Design (www.revkit.org)
 * Copyright (C) 2009-2014  The RevKit Developers <revkit@informatik.uni-bremen.de>
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
 * @file lnn_optimization.hpp
 *
 * @brief Linear nearest Neighbor
 *
 * @author Aaron Lye
 * @since  2.0
 */

#ifndef LNN_OPTIMIZATION_HPP
#define LNN_OPTIMIZATION_HPP

#include <core/properties.hpp>

#include <reversible/circuit.hpp>
#include <reversible/truth_table.hpp>

#include <reversible/optimization/optimization.hpp>

namespace cirkit
{

/**
 * @brief Linear nearest Neighbor
 *
 * Algorith implements a linear nearest neighbor approach
 *
 * @since 2.0
 */
bool lnn_optimization( circuit& circ, const circuit& base, properties::ptr settings = properties::ptr(), properties::ptr statistics = properties::ptr() );

/**
 * @brief Functor for the lnn_optimization algorithm
 *
 * @param settings Settings (see lnn_optimization)
 * @param statistics Statistics (see lnn_optimization)
 *
 * @return A functor which complies with the optimization_func interface
 *
 * @since 2.0
 */
optimization_func lnn_optimization_func( properties::ptr settings = properties::ptr( new properties() ), properties::ptr statistics = properties::ptr( new properties() ) );

}

#endif /* LNN_OPTIMIZATION_HPP */

// Local Variables:
// c-basic-offset: 2
// End:
