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
 * @file bdd_level_approximation.hpp
 *
 * @brief BDD-based approximation using level operations
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef BDD_LEVEL_APPROXIMATION_HPP
#define BDD_LEVEL_APPROXIMATION_HPP

#include <vector>

#include <core/properties.hpp>
#include <classical/dd/bdd.hpp>

namespace cirkit
{

enum class bdd_level_approximation_mode { round_down, round_up, round, cof0, cof1 };

std::vector<bdd> bdd_level_approximation( const std::vector<bdd>& fs, bdd_level_approximation_mode mode, unsigned level,
                                          const properties::ptr& settings = properties::ptr(),
                                          const properties::ptr& statistics = properties::ptr() );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
