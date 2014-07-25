/* CirKit: A circuit toolkit
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
 * @file mincut.hpp
 *
 * @brief Min-Cut algorithms in AIGs
 *
 * @author Mathias Soeken
 * @since  2.0
 */

#ifndef MINCUT_HPP
#define MINCUT_HPP

#include <list>

#include <boost/graph/adjacency_list.hpp>
#include <boost/optional.hpp>

#include <core/functor.hpp>

#include <classical/aig.hpp>

namespace cirkit
{

  /******************************************************************************
   * Types                                                                      *
   ******************************************************************************/

  enum mc_vertex_type_t { mc_node, mc_source, mc_target };

  struct mc_vertex_info_t
  {
    mc_vertex_type_t          type = mc_node;
    boost::optional<aig_node> original_node;
  };

  struct mc_edge_info_t
  {
    boost::optional<aig_node> original_node;
    double                    original_capacity;
  };

  typedef boost::adjacency_list_traits<boost::vecS, boost::vecS, boost::bidirectionalS> mc_traits_t;
  typedef boost::property<boost::vertex_color_t, boost::default_color_type,
          boost::property<boost::vertex_distance_t, long,
          boost::property<boost::vertex_predecessor_t, mc_traits_t::edge_descriptor,
          boost::property<boost::vertex_name_t, mc_vertex_info_t>>>> mc_vertex_properties_t;
  typedef boost::property<boost::edge_capacity_t, double,
          boost::property<boost::edge_residual_capacity_t, double,
          boost::property<boost::edge_reverse_t, mc_traits_t::edge_descriptor,
          boost::property<boost::edge_name_t, mc_edge_info_t>>>> mc_edge_properties_t;
  typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, mc_vertex_properties_t, mc_edge_properties_t> mc_graph_t;

  typedef boost::graph_traits<mc_graph_t>::vertex_descriptor mc_vertex_t;
  typedef boost::graph_traits<mc_graph_t>::edge_descriptor mc_edge_t;

  /******************************************************************************
   * Functors                                                                   *
   ******************************************************************************/

  typedef functor<bool(std::list<std::list<aig_function>>&, aig_graph&, unsigned count)> mincut_by_edge_func;
  typedef functor<bool(std::list<std::list<aig_node>>&, aig_graph&, unsigned count)> mincut_by_node_func;

}

#endif

// Local Variables:
// c-basic-offset: 2
// End:
