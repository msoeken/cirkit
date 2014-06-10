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
 * @file young_subgroup_synthesis.hpp
 *
 * @brief Young subgroup synthesis
 *
 * @author Nabila Abdessaied
 * @author Mathias Soeken
 *
 * @since  2.0
 */

#ifndef YOUNG_SUBGROUP_SYNTHESIS
#define YOUNG_SUBGROUP_SYNTHESIS

#include <core/circuit.hpp>
#include <core/properties.hpp>
#include <core/truth_table.hpp>

#include <algorithms/synthesis/synthesis.hpp>

namespace revkit
{

bool young_subgroup_synthesis(circuit& circ, const binary_truth_table& spec, properties::ptr settings = properties::ptr(), properties::ptr statistics = properties::ptr());

truth_table_synthesis_func young_subgroup_synthesis_func(properties::ptr settings = properties::ptr(new properties()), properties::ptr statistics = properties::ptr(new properties()));

}

#endif

// Local Variables:
// c-basic-offset: 2
// End:
