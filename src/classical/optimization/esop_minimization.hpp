/* RevKit: A Toolkit for Reversible Circuit Design (www.revkit.org)
 * Copyright (C) 2009-2013  The RevKit Developers <revkit@informatik.uni-bremen.de>
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
 * @brief Generation of exact PSDKRO expression from PLA
 *
 * @author Mathias Soeken
 * @since  2.0
 */

#ifndef ESOP_MINIMIZATION_HPP
#define ESOP_MINIMIZATION_HPP

#include <string>

#include <cudd.h>

#include <core/properties.hpp>

namespace revkit
{

void esop_minimization( DdManager * cudd, DdNode * f,
                      properties::ptr settings = properties::ptr(),
                      properties::ptr statistics = properties::ptr() );
void esop_minimization( const std::string& filename,
                      properties::ptr settings = properties::ptr(),
                      properties::ptr statistics = properties::ptr() );

void test_change_performance();

}

#endif

// Local Variables:
// c-basic-offset: 2
// End:
