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
 * @file lut_graph.hpp
 *
 * @brief LUT Graph
 *
 * @author Mathias Soeken
 * @since  2.2
 */

#ifndef LUT_GRAPH_HPP
#define LUT_GRAPH_HPP

#include <iostream>
#include <unordered_map>

#include <core/utils/dirty.hpp>
#include <core/utils/graph_utils.hpp>

#include <boost/dynamic_bitset.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/properties.hpp>
#include <range/v3/iterator_range.hpp>

namespace boost
{

enum vertex_lut_type_t { vertex_lut_type };
BOOST_INSTALL_PROPERTY(vertex, lut_type);

enum vertex_lut_t { vertex_lut };
BOOST_INSTALL_PROPERTY(vertex, lut);

}

namespace cirkit
{
using boost::adjacency_list;
using boost::default_color_type;
using boost::directedS;
using boost::edge_name_t;
using boost::property;
using boost::vecS;
using boost::vertex_color_t;
using boost::vertex_name_t;
using boost::vertex_lut_t;
using boost::vertex_lut_type_t;

enum class lut_type_t { pi, po, gnd, vdd, internal };

/******************************************************************************
 * LUT graph                                                                  *
 ******************************************************************************/

using lut_graph_vertex_properties_t = property<vertex_name_t, std::string,
                                      property<vertex_lut_t, std::string,
                                      property<vertex_lut_type_t, lut_type_t>>>;
using lut_graph_t  = digraph_t<lut_graph_vertex_properties_t>;
using lut_vertex_t = vertex_t<lut_graph_t>;
using lut_edge_t   = edge_t<lut_graph_t>;

unsigned lut_graph_lut_count( const lut_graph_t& g );

class lut_graph
{
public:
  using graph_t = lut_graph_t;
  using node_t = vertex_t<graph_t>;

  using input_vec_t  = std::vector<std::pair<node_t, std::string>>;
  using output_vec_t = std::vector<std::pair<node_t, std::string>>;

  using vertex_range_t = ranges::iterator_range<boost::graph_traits<lut_graph_t>::vertex_iterator>;
  using edge_range_t   = ranges::iterator_range<boost::graph_traits<lut_graph_t>::edge_iterator>;

  using name_property_map_t      = boost::property_map<graph_t, boost::vertex_name_t>::type;
  using lut_property_map_t       = boost::property_map<graph_t, boost::vertex_lut_t>::type;
  using gate_type_property_map_t = boost::property_map<graph_t, boost::vertex_lut_type_t>::type;

  lut_graph( const std::string& name = std::string() );

  lut_graph& operator=( const lut_graph& other )
  {
    g = other.g;
    gnd = other.gnd;
    vdd = other.vdd;

    _name = other._name;
    _inputs = other._inputs;
    _outputs = other._outputs;
    _input_to_id = other._input_to_id;

    _names = boost::get( boost::vertex_name, g );
    _types = boost::get( boost::vertex_lut_type, g );
    _luts = boost::get( boost::vertex_lut, g );
    for ( const auto& v : boost::make_iterator_range( boost::vertices( g ) ) )
    {
      _names[ v ] = other._names[ v ];
      _types[ v ] = other._types[ v ];
      _luts[ v ] = other._luts[ v ];
    }

    // fanout = other.fanout;
    // parentss = other.parentss;
    // levels = other.levels;

    _num_lut = other._num_lut;

    // ref_count = other.ref_count;
    // marks = other.marks;

    return *this;
  }
  void compute_fanout() const;
  void compute_parents() const;
  void compute_levels() const;
  void compute_sections() const;

  node_t get_constant( bool value ) const;
  node_t create_pi( const std::string& name );
  void create_po( const node_t& n, const std::string& name );
  void delete_po( unsigned index );
  node_t create_lut( const std::string& lut, const std::vector<node_t>& ops, const std::string& name = std::string() );

  unsigned fanin_count( const node_t& n ) const;
  unsigned fanout_count( const node_t& n ) const;
  const std::vector<node_t>& parents( const node_t& n ) const;
  const boost::dynamic_bitset<>& section( const node_t& n ) const;
  unsigned level( const node_t& n ) const;

  bool is_input( const node_t& n ) const;
  bool is_lut( const node_t& n ) const;

  const std::string& name() const;
  void set_name( const std::string& name );
  std::size_t size() const;
  unsigned num_gates() const;
  const graph_t& graph() const;
  graph_t& graph();
  const input_vec_t& inputs() const;
  const output_vec_t& outputs() const;
  input_vec_t& inputs();
  output_vec_t& outputs();
  const std::string& input_name( const node_t& n ) const;
  const unsigned input_index( const node_t& n ) const;
  std::vector<node_t> children( const node_t& n ) const;
  vertex_range_t nodes() const;
  edge_range_t edges() const;
  std::vector<node_t> topological_nodes() const;

  inline const name_property_map_t& names() { return _names; }
  inline const name_property_map_t& names() const { return _names; }
  inline const lut_property_map_t& luts() { return _luts; }
  inline const lut_property_map_t& luts() const { return _luts; }
  inline const gate_type_property_map_t& types() { return _types; }
  inline const gate_type_property_map_t& types() const { return _types; }

  inline unsigned num_luts() const { return _num_lut; }

  /* ref counting */
  void init_refs() const;
  unsigned get_ref( const lut_vertex_t& n ) const;
  unsigned inc_ref( const lut_vertex_t& n ) const;
  unsigned dec_ref( const lut_vertex_t& n ) const;
  void inc_output_refs() const;

  /* marking */
  void init_marks() const;
  bool is_marked( const lut_vertex_t& n ) const;
  void mark( const lut_vertex_t& n ) const;

  void mark_as_modified() const;

private:
  graph_t g;
  node_t gnd;
  node_t vdd;

  std::string  _name;
  input_vec_t  _inputs;
  output_vec_t _outputs;
  std::unordered_map<node_t, unsigned> _input_to_id;

  name_property_map_t      _names;
  gate_type_property_map_t _types;
  lut_property_map_t       _luts;

  /* additional network information */
  mutable dirty<std::vector<unsigned>>                fanout;
  mutable dirty<std::vector<std::vector<node_t>>>     parentss;
  mutable dirty<std::vector<unsigned>>                levels;
  mutable dirty<std::vector<boost::dynamic_bitset<>>> sections;

  /* network settings and stats */
  unsigned                                _num_lut = 0u;

  /* utilities */
  mutable std::vector<unsigned>                   ref_count;
  mutable boost::dynamic_bitset<>                 marks;
};

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
