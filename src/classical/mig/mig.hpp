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
 * @file mig.hpp
 *
 * @brief Majority Inverter Graphs
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef MIG_HPP
#define MIG_HPP

#include <core/properties.hpp>
#include <core/utils/graph_utils.hpp>
#include <classical/traits.hpp>

namespace cirkit
{

namespace detail
{
using mig_traits_t = digraph_traits_t;
}

struct mig_function
{
  detail::mig_traits_t::vertex_descriptor node;
  bool                                    complemented;

  inline mig_function operator!() const
  {
    return {node, !complemented};
  }

  inline bool operator==( const mig_function& other ) const
  {
    return node == other.node && complemented == other.complemented;
  }

  inline bool operator<( const mig_function& other ) const
  {
    return node < other.node || ( node == other.node && complemented < other.complemented );
  }

  inline mig_function operator^( bool value ) const
  {
    return {node, complemented != value };
  }
};

struct mig_graph_info
{
  std::string                                                                  model_name;
  detail::mig_traits_t::vertex_descriptor                                      constant;
  bool                                                                         constant_used = false;
  std::map<detail::mig_traits_t::vertex_descriptor, std::string>               node_names;
  std::vector<std::pair<mig_function, std::string> >                           outputs;
  std::vector<detail::mig_traits_t::vertex_descriptor>                         inputs;
  std::map<std::tuple<mig_function, mig_function, mig_function>, mig_function> strash;
};

namespace detail
{
using mig_vertex_properties_t = boost::no_property;
using mig_edge_properties_t   = boost::property<boost::edge_complement_t, bool>;
using mig_graph_properties_t  = boost::property<boost::graph_name_t, mig_graph_info>;
}

using mig_graph = digraph_t<detail::mig_vertex_properties_t,
                            detail::mig_edge_properties_t,
                            detail::mig_graph_properties_t>;

using mig_node = vertex_t<mig_graph>;
using mig_edge = edge_t<mig_graph>;

void mig_initialize( mig_graph& mig, const std::string& model_name = std::string() );
mig_function mig_get_constant( mig_graph& mig, bool value );
bool mig_is_constant_used( const mig_graph& mig );
mig_function mig_create_pi( mig_graph& mig, const std::string& name );
void mig_create_po( mig_graph& mig, const mig_function& f, const std::string& name );
mig_function mig_create_maj( mig_graph& mig, const mig_function& a, const mig_function& b, const mig_function& c );
mig_function mig_create_and( mig_graph& mig, const mig_function& a, const mig_function& b );
mig_function mig_create_or( mig_graph& mig, const mig_function& a, const mig_function& b );
mig_function mig_create_xor( mig_graph& mig, const mig_function& a, const mig_function& b );

void write_dot( const mig_graph& mig, std::ostream& os, const properties::ptr& settings = properties::ptr() );
void write_dot( const mig_graph& mig, const std::string& filename, const properties::ptr& settings = properties::ptr() );

mig_function mig_to_function( const mig_graph& mig, const mig_edge& edge );

template<>
struct circuit_traits<mig_graph>
{
  using node           = mig_node;
  using edge           = mig_edge;
  using node_color_map = std::map<mig_node, boost::default_color_type>;
};

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
