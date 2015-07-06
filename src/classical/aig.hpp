/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
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
 * @author Heinz Riener
 * @since  2.0
 */

#ifndef AIG_HPP
#define AIG_HPP

#include <iostream>
#include <map>
#include <vector>

#include <boost/graph/adjacency_list.hpp>
#include <boost/property_map/property_map.hpp>

#include <core/properties.hpp>
#include <core/utils/graph_utils.hpp>
#include <classical/traits.hpp>

namespace cirkit
{

namespace detail
{
  using traits_t = boost::adjacency_list_traits<boost::vecS, boost::vecS, boost::directedS>;
}

struct aig_function
{
  detail::traits_t::vertex_descriptor node;
  bool                                complemented;

  inline aig_function operator!() const
  {
    return {node, !complemented};
  }

  inline bool operator==( const aig_function& other ) const
  {
    return node == other.node && complemented == other.complemented;
  }

  inline bool operator!=( const aig_function& other ) const
  {
    return !( this->operator==(other) );
  }

  inline bool operator<( const aig_function& other ) const
  {
    return node < other.node || ( node == other.node && complemented < other.complemented );
  }
};

struct aig_graph_info
{
  std::string                                                   model_name;
  detail::traits_t::vertex_descriptor                           constant;
  bool                                                          constant_used = false;
  bool                                                          enable_strashing = true;
  bool                                                          enable_local_optimization = true;
  std::map<detail::traits_t::vertex_descriptor, std::string>    node_names;
  std::vector<std::pair<aig_function, std::string> >            outputs;
  std::vector<detail::traits_t::vertex_descriptor>              inputs;
  std::vector<aig_function>                                     cos;
  std::vector<detail::traits_t::vertex_descriptor>              cis;
  std::map<std::pair<aig_function, aig_function>, aig_function> strash;
  std::map<aig_function, aig_function>                          latch;
};

namespace detail
{

using vertex_properties_t = boost::property<boost::vertex_name_t, unsigned,
                            boost::property<boost::vertex_annotation_t, std::map<std::string, std::string> > >;
using edge_properties_t = boost::property<boost::edge_complement_t, bool>;
using graph_properties_t = boost::property<boost::graph_name_t, aig_graph_info>;

}

using aig_graph = digraph_t<detail::vertex_properties_t,
                            detail::edge_properties_t,
                            detail::graph_properties_t>;

using aig_node = vertex_t<aig_graph>;
using aig_edge = edge_t<aig_graph>;

void aig_initialize( aig_graph& aig, const std::string& model_name = std::string() );
aig_function aig_get_constant( aig_graph& aig, bool value );
bool aig_is_constant_used( const aig_graph& aig );
aig_function aig_create_pi( aig_graph& aig, const std::string& name );
void aig_create_po( aig_graph& aig, const aig_function& f, const std::string& name );
aig_function aig_create_ci( aig_graph& aig, const std::string& name );
void aig_create_co( aig_graph& aig, const aig_function& f );
aig_function aig_create_and( aig_graph& aig, const aig_function& left, const aig_function& right );
aig_function aig_create_nand( aig_graph& aig, const aig_function& left, const aig_function& right );
aig_function aig_create_or( aig_graph& aig, const aig_function& left, const aig_function& right );
aig_function aig_create_nor( aig_graph& aig, const aig_function& left, const aig_function& right );
aig_function aig_create_xor( aig_graph& aig, const aig_function& left, const aig_function& right );
aig_function aig_create_ite( aig_graph& aig, const aig_function& cond, const aig_function& t, const aig_function& e );
aig_function aig_create_implies( aig_graph& aig, const aig_function& a, const aig_function& b );
aig_function aig_create_maj( aig_graph& aig, const aig_function& a, const aig_function& b, const aig_function& c );
aig_function aig_create_lat( aig_graph& aig, const aig_function& in, const std::string& name );

aig_function aig_create_nary_and( aig_graph& aig, const std::vector< aig_function >& v );
aig_function aig_create_nary_nand( aig_graph& aig, const std::vector< aig_function >& v );
aig_function aig_create_nary_or( aig_graph& aig, const std::vector< aig_function >& v );
aig_function aig_create_nary_nor( aig_graph& aig, const std::vector< aig_function >& v );
aig_function aig_create_nary_xor( aig_graph& aig, const std::vector< aig_function >& v );

void write_dot( const aig_graph& aig, std::ostream& os, const properties::ptr& settings = properties::ptr() );
void write_dot( const aig_graph& aig, const std::string& filename, const properties::ptr& settings = properties::ptr() );

unsigned aig_to_literal( const aig_graph& aig, const aig_function& f );
unsigned aig_to_literal( const aig_graph& aig, const aig_node& node );
aig_function aig_to_function( const aig_graph& aig, const aig_edge& edge );

std::ostream& operator<<( std::ostream& os, const aig_function &f );

template<>
struct circuit_traits<aig_graph>
{
  using node           = aig_node;
  using edge           = aig_edge;
  using node_color_map = std::map<aig_node, boost::default_color_type>;
};

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
