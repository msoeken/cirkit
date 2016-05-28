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
 * @file traits.hpp
 *
 * @brief Cut enumeration traits for MIGs
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef CUTS_TRAITS_HPP
#define CUTS_TRAITS_HPP

#include <classical/mig/mig.hpp>
#include <classical/mig/mig_simulate.hpp>
#include <classical/utils/cut_enumeration_traits.hpp>

namespace cirkit
{

namespace detail
{

template<>
struct cut_enumeration_circuit_traits<mig_graph>
{
  template<typename T>
  using simulator = mig_simulator<T>;
  using tt_simulator = mig_tt_simulator;
  template<typename T>
  using partial_node_assignment_simulator = mig_partial_node_assignment_simulator<T>;

  template<typename CutsType>
  static CutsType simulate_node( const mig_graph& mig,
                                 const mig_node& node,
                                 const mig_simulator<CutsType>& simulator,
                                 mig_node_color_map& colors,
                                 std::map<mig_node, CutsType>& node_values )
  {
    return simulate_mig_node( mig, node, simulator, colors, node_values );
  }

  // template<typename CutsType>
  // static const std::function<CutsType(const mig_graph&,
  //                                     const mig_node&,
  //                                     const mig_simulator<CutsTpe>&,
  //                                     mig_node_color_map&,
  //                                     std::map<mig_node, CutsType>&)> simulate_node;

  template<typename T>
  static T simulate_node( const mig_graph& mig,
                          const mig_node& node,
                          const mig_simulator<T>& simulator )
  {
    return simulate_mig_node( mig, node, simulator );
  }


  // static const std::function<tt(const mig_graph&,
  //                               const mig_node&,
  //                               const mig_simulator<tt>&)> simulate_node2;

  // static const std::function<unsigned(const mig_graph&,
  //                                     const mig_node&,
  //                                     const mig_simulator<unsigned>&)> simulate_node3;
};

}

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
