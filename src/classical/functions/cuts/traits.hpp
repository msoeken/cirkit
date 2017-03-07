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
