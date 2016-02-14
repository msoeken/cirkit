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
 * @file cut_enumeration_traits.hpp
 *
 * @brief Cut enumeration traits and global definitions
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef CUT_ENUMERATION_TRAITS_HPP
#define CUT_ENUMERATION_TRAITS_HPP

#include <map>
#include <vector>

#include <classical/aig.hpp>
#include <classical/functions/simulate_aig.hpp>
#include <classical/utils/truth_table_utils.hpp>

namespace cirkit
{

namespace detail
{

/******************************************************************************
 * Implementation specific traits                                             *
 ******************************************************************************/

template<typename T>
struct cut_enumeration_circuit_traits {};

template<>
struct cut_enumeration_circuit_traits<aig_graph>
{
  template<typename T>
  using simulator = aig_simulator<T>;
  using tt_simulator = cirkit::tt_simulator;
  template<typename T>
  using partial_node_assignment_simulator = aig_partial_node_assignment_simulator<T>;

  template<typename CutsType>
  static CutsType simulate_node( const aig_graph& aig,
                                 const aig_node& node,
                                 const aig_simulator<CutsType>& simulator,
                                 aig_node_color_map& colors,
                                 std::map<aig_node, CutsType>& node_values )
  {
    return simulate_aig_node( aig, node, simulator, colors, node_values );
  }
  //  static const std::function<CutsType(const aig_graph&,
  //                                      const aig_node&,
  //                                      const aig_simulator<CutsType>&,
  //                                      aig_node_color_map&,
  //                                      std::map<aig_node, CutsType>&)> simulate_node;

  template<typename T>
  static T simulate_node( const aig_graph& aig,
                          const aig_node& node,
                          const aig_simulator<T>& simulator )
  {
    return simulate_aig_node( aig, node, simulator );
  }

  // static const std::function<tt(const aig_graph&,
  //                               const aig_node&,
  //                               const aig_simulator<tt>&)> simulate_node2;

  // static const std::function<unsigned(const aig_graph&,
  //                                     const aig_node&,
  //                                     const aig_simulator<unsigned>&)> simulate_node3;
};

}

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
