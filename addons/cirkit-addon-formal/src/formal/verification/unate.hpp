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
 * @file unate.hpp
 *
 * @brief Check for unateness
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef UNATE_HPP
#define UNATE_HPP

#include <vector>

#include <boost/dynamic_bitset.hpp>

#include <core/properties.hpp>
#include <classical/aig.hpp>

namespace cirkit
{

enum class unate_kind { binate, unate_pos, unate_neg, independent };

/**
 * The return value contains all the information for each output/input pair
 * ordered first by outputs, then by inputs, each pair takes 2 bits in the bitset:
 *
 * 00: binate
 * 01: unate_pos
 * 10: unate_neg
 * 11: independent
 */
boost::dynamic_bitset<> unateness_naive( const aig_graph& aig,
                                         const properties::ptr& settings = properties::ptr(),
                                         const properties::ptr& statistics = properties::ptr() );

boost::dynamic_bitset<> unateness_split( const aig_graph& aig,
                                         const properties::ptr& settings = properties::ptr(),
                                         const properties::ptr& statistics = properties::ptr() );

boost::dynamic_bitset<> unateness_split_parallel( const aig_graph& aig,
                                                  const properties::ptr& settings = properties::ptr(),
                                                  const properties::ptr& statistics = properties::ptr() );

boost::dynamic_bitset<> unateness_split_inputs_parallel( const aig_graph& aig,
                                                         const properties::ptr& settings = properties::ptr(),
                                                         const properties::ptr& statistics = properties::ptr() );

boost::dynamic_bitset<> unateness( const aig_graph& aig,
                                   const properties::ptr& settings = properties::ptr(),
                                   const properties::ptr& statistics = properties::ptr() );

unate_kind get_unateness_kind( const boost::dynamic_bitset<>& u, unsigned po, unsigned pi, unsigned num_pis );
unate_kind get_unateness_kind( const boost::dynamic_bitset<>& u, unsigned po, unsigned pi, const aig_graph_info& info );
unate_kind get_unateness_kind( const boost::dynamic_bitset<>& u, unsigned po, unsigned pi, const aig_graph& aig );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
