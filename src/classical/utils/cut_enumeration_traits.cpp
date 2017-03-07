/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2017  EPFL
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
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
