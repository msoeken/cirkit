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
 * @file bdd_to_truth_table.hpp
 *
 * @brief Creates a truth table from a BDD
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef BDD_TO_TRUTH_TABLE_HPP
#define BDD_TO_TRUTH_TABLE_HPP

#include <vector>

#include <core/properties.hpp>
#include <classical/dd/bdd.hpp>
#include <classical/utils/truth_table_utils.hpp>

namespace cirkit
{

enum class bdd_to_truth_table_method { dfs, visit };

tt bdd_to_truth_table( const bdd& b,
                       const properties::ptr& settings = properties::ptr(),
                       const properties::ptr& statistics = properties::ptr() );

std::vector<unsigned> bdds_to_truth_table_unsigned( const std::vector<bdd>& fs,
                                                    const properties::ptr& settings = properties::ptr(),
                                                    const properties::ptr& statistics = properties::ptr() );

std::vector<int> bdds_to_truth_table_signed( const std::vector<bdd>& fs,
                                             const properties::ptr& settings = properties::ptr(),
                                             const properties::ptr& statistics = properties::ptr() );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
