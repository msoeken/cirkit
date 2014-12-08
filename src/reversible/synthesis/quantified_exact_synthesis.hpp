/* RevKit (www.revkit.org)
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
 * @file quantified_exact_synthesis.hpp
 *
 * @brief Quantified exact synthesis using BDDs
 *
 * @author Mathias Soeken
 * @author Hoang M. Le
 * @since  2.1
 */

#ifndef QUANTIFIED_EXACT_SYNTHESIS_HPP
#define QUANTIFIED_EXACT_SYNTHESIS_HPP

#include <core/properties.hpp>
#include <reversible/circuit.hpp>
#include <reversible/truth_table.hpp>

#include <reversible/synthesis/synthesis.hpp>

namespace cirkit
{

bool quantified_exact_synthesis( circuit& circ, const binary_truth_table& spec,
                                 properties::ptr settings = properties::ptr(),
                                 properties::ptr statistics = properties::ptr() );


truth_table_synthesis_func quantified_exact_synthesis_func( properties::ptr settings = std::make_shared<properties>(),
                                                            properties::ptr statistics = std::make_shared<properties>() );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
