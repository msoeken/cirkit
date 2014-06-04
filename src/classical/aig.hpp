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
 * @file aig.hpp
 *
 * @brief AIG package
 *
 * @author Mathias Soeken
 * @since  2.0
 */

#ifndef AIG_HPP
#define AIG_HPP

#include <iostream>
#include <map>
#include <vector>

#include <boost/graph/adjacency_list.hpp>

namespace boost
{
  enum edge_polarity_t { edge_polarity };
  BOOST_INSTALL_PROPERTY(edge, polarity);
}

namespace revkit
{
  typedef boost::adjacency_list_traits<boost::vecS, boost::vecS, boost::directedS> traits_t;
  typedef std::pair<traits_t::vertex_descriptor, bool> aig_function; /* the second type is for the polarity */

  struct aig_graph_info
  {
    std::map<traits_t::vertex_descriptor, std::string> node_names;
    std::vector<std::pair<aig_function, std::string> > outputs;
    std::vector<traits_t::vertex_descriptor> inputs;
    std::map<std::pair<aig_function, aig_function>, aig_function> strash;
  };

  typedef boost::property<boost::vertex_name_t, unsigned,
          boost::property<boost::vertex_color_t, boost::default_color_type,
          boost::property<boost::vertex_distance_t, long,
          boost::property<boost::vertex_predecessor_t, traits_t::edge_descriptor> > > > vertex_properties_t;
  typedef boost::property<boost::edge_capacity_t, double,
          boost::property<boost::edge_residual_capacity_t, double,
          boost::property<boost::edge_reverse_t, traits_t::edge_descriptor,
          boost::property<boost::edge_polarity_t, bool> > > > edge_properties_t;
  typedef boost::property<boost::graph_name_t, aig_graph_info> graph_properties_t;
  typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, vertex_properties_t, edge_properties_t, graph_properties_t> aig_graph;

  typedef boost::graph_traits<aig_graph>::vertex_descriptor aig_node;
  typedef boost::graph_traits<aig_graph>::edge_descriptor aig_edge;

  void aig_initialize( aig_graph& aig );
  aig_function aig_get_constant( aig_graph& aig, bool value );
  aig_function aig_create_pi( aig_graph& aig, const std::string& name );
  void aig_create_po( aig_graph& aig, const aig_function& f, const std::string& name );
  aig_function aig_create_and( aig_graph& aig, const aig_function& left, const aig_function& right );
  aig_function aig_create_or( aig_graph& aig, const aig_function& left, const aig_function& right );
  aig_function aig_create_xor( aig_graph& aig, const aig_function& left, const aig_function& right );
  aig_function aig_create_ite( aig_graph& aig, const aig_function& cond, const aig_function& t, const aig_function& e );
  aig_function aig_create_maj( aig_graph& aig, const aig_function& a, const aig_function& b, const aig_function& c );

  void write_dot( std::ostream& os, const aig_graph& aig );

  aig_function operator!( const aig_function& f );

}

#endif

// Local Variables:
// c-basic-offset: 2
// End:
