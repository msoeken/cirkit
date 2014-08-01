/* RevKit (www.rekit.org)
 * Copyright (C) 2009-2014  University of Bremen
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
 * @file esop_minimization.hpp
 *
 * @brief ESOP minimization
 *
 * @author Mathias Soeken
 * @since  2.0
 */

#ifndef ESOP_MINIMIZATION_HPP
#define ESOP_MINIMIZATION_HPP

#include <functional>
#include <string>

#include <boost/dynamic_bitset.hpp>

#include <cudd.h>

#include <core/properties.hpp>
#include <classical/optimization/optimization.hpp>

namespace cirkit
{

/**
 * @brief ESOP minimization
 *
 * This convenience function takes the BDD node that contains
 * the function to be optimized directly.
 *
 * @author Mathias Soeken
 */
void esop_minimization( DdManager * cudd, DdNode * f,
                        properties::ptr settings = properties::ptr(),
                        properties::ptr statistics = properties::ptr() );

/**
 * @brief ESOP minimization
 *
 * This optimization algorithm is an implementation of the paper
 * [A. Mishchenko and M. Perkowski, Reed Muller Workshop 5 (2001)].
 * The paper has also been implemented in the tool EXORCISM-4 by the
 * same authors.
 *
 * In comparison to EXORCISM-4 this algorithm does not support functions
 * with mulitple outputs.
 *
 * @author Mathias Soeken
 */
void esop_minimization( const std::string& filename,
                        properties::ptr settings = properties::ptr(),
                        properties::ptr statistics = properties::ptr() );

dd_based_esop_optimization_func dd_based_esop_minimization_func( properties::ptr settings = properties::ptr( new properties() ),
                                                                 properties::ptr statistics = properties::ptr( new properties() ) );

pla_based_esop_optimization_func pla_based_esop_minimization_func( properties::ptr settings = properties::ptr( new properties() ),
                                                                   properties::ptr statistics = properties::ptr( new properties() ) );

void test_change_performance();

}

#endif

// Local Variables:
// c-basic-offset: 2
// End:
