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

#include "cut_enumeration_traits.hpp"

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

namespace detail
{

// const std::function<structural_cut(const aig_graph&, const aig_node&, const aig_simulator<structural_cut>&, aig_node_color_map&, std::map<aig_node, structural_cut>&)>
// cut_enumeration_circuit_traits<aig_graph>::simulate_node = []( const aig_graph& aig, const aig_node& node,
//                                                                const aig_simulator<structural_cut>& simulator,
//                                                                aig_node_color_map& colors,
//                                                                std::map<aig_node, structural_cut>& node_values )
// {
//   return simulate_aig_node( aig, node, simulator, colors, node_values );
// };

// const std::function<tt(const aig_graph&, const aig_node&, const aig_simulator<tt>&)>
// cut_enumeration_circuit_traits<aig_graph>::simulate_node2 = []( const aig_graph& aig, const aig_node& node,
//                                                                 const aig_simulator<tt>& simulator )
// {
//   return simulate_aig_node( aig, node, simulator );
// };

// const std::function<unsigned(const aig_graph&, const aig_node&, const aig_simulator<unsigned>&)>
// cut_enumeration_circuit_traits<aig_graph>::simulate_node3 = []( const aig_graph& aig, const aig_node& node,
//                                                                 const aig_simulator<unsigned>& simulator )
// {
//   return simulate_aig_node( aig, node, simulator );
// };

// const std::function<structural_cut(const mig_graph&, const mig_node&, const mig_simulator<structural_cut>&, mig_node_color_map&, std::map<mig_node, structural_cut>&)>
// cut_enumeration_circuit_traits<mig_graph>::simulate_node = []( const mig_graph& mig, const mig_node& node,
//                                                                const mig_simulator<structural_cut>& simulator,
//                                                                mig_node_color_map& colors,
//                                                                std::map<mig_node, structural_cut>& node_values )
// {
//   return simulate_mig_node( mig, node, simulator, colors, node_values );
// };

// const std::function<tt(const mig_graph&, const mig_node&, const mig_simulator<tt>&)>
// cut_enumeration_circuit_traits<mig_graph>::simulate_node2 = []( const mig_graph& mig, const mig_node& node,
//                                                                 const mig_simulator<tt>& simulator )
// {
//   return simulate_mig_node( mig, node, simulator );
// };

// const std::function<unsigned(const mig_graph&, const mig_node&, const mig_simulator<unsigned>&)>
// cut_enumeration_circuit_traits<mig_graph>::simulate_node3 = []( const mig_graph& mig, const mig_node& node,
//                                                                 const mig_simulator<unsigned>& simulator )
// {
//   return simulate_mig_node( mig, node, simulator );
// };

}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
