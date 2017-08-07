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
 * @file xmg.hpp
 *
 * @brief XOR Majority Graph
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef XMG_HPP
#define XMG_HPP

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include <boost/dynamic_bitset.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <range/v3/iterator_range.hpp>

#include <core/utils/dirty.hpp>
#include <core/utils/graph_utils.hpp>
#include <core/utils/hash_utils.hpp>

namespace cirkit
{

using xmg_graph_t = digraph_t<boost::no_property, boost::property<boost::edge_complement_t, bool>>;
using xmg_node    = vertex_t<xmg_graph_t>;
using xmg_edge    = edge_t<xmg_graph_t>;

class xmg_function
{
public:
  xmg_function( xmg_node node = 0, bool complemented = false );

  bool operator==( const xmg_function& other ) const;
  bool operator!=( const xmg_function& other ) const;
  bool operator<( const xmg_function& other ) const;
  bool operator>( const xmg_function& other ) const;

  xmg_function operator!() const;
  xmg_function operator^( bool value ) const;

public:
  xmg_node node;
  bool     complemented;
};

}

namespace std
{

template<>
struct hash<cirkit::xmg_function>
{
  inline std::size_t operator()( const cirkit::xmg_function& f ) const
  {
    return ( f.node << 1u ) + static_cast<int>( f.complemented );
  }
};

}

namespace cirkit
{

class xmg_cover;
class xmg_bitmarks;

class xmg_graph
{
public:
  using graph_t = xmg_graph_t;
  using node_t  = vertex_t<graph_t>;

  using input_vec_t  = std::vector<std::pair<node_t, std::string>>;
  using output_vec_t = std::vector<std::pair<xmg_function, std::string>>;

  using vertex_range_t = ranges::iterator_range<boost::graph_traits<xmg_graph_t>::vertex_iterator>;
  using edge_range_t   = ranges::iterator_range<boost::graph_traits<xmg_graph_t>::edge_iterator>;

  using complement_property_map_t = boost::property_map<graph_t, boost::edge_complement_t>::type;

public:
  xmg_graph( const std::string& name = std::string() );

  void compute_fanout();
  void compute_parents();
  void compute_levels();

  xmg_function get_constant( bool value ) const;
  xmg_function create_pi( const std::string& name );
  void create_po( const xmg_function& f, const std::string& name );
  void delete_po( unsigned index );
  xmg_function create_maj( const xmg_function& a, const xmg_function& b, const xmg_function& c );
  xmg_function create_xor( const xmg_function& a, const xmg_function& b );
  xmg_function create_and( const xmg_function& a, const xmg_function& b );
  xmg_function create_or( const xmg_function& a, const xmg_function& b );
  xmg_function create_ite( const xmg_function& c, const xmg_function& t, const xmg_function& e );
  xmg_function create_nary_and( const std::vector<xmg_function>& ops );
  xmg_function create_nary_or( const std::vector<xmg_function>& ops );

  unsigned fanin_count( node_t n ) const;
  unsigned fanout_count( node_t n ) const;
  const std::vector<node_t>& parents( node_t n ) const;
  unsigned level( node_t n ) const;

  bool is_input( node_t n ) const;
  bool is_maj( node_t n ) const;
  bool is_pure_maj( node_t n ) const;
  bool is_xor( node_t n ) const;

  const std::string& name() const;
  void set_name( const std::string& name );
  std::size_t size() const;
  unsigned num_gates() const;
  unsigned num_maj() const;
  unsigned num_xor() const;
  const graph_t& graph() const;
  graph_t& graph();
  const input_vec_t& inputs() const;
  const output_vec_t& outputs() const;
  input_vec_t& inputs();
  output_vec_t& outputs();
  const std::string& input_name( xmg_node n ) const;
  const unsigned input_index( xmg_node n ) const;
  std::vector<xmg_function> children( xmg_node n ) const;
  vertex_range_t nodes() const;
  edge_range_t edges() const;
  std::vector<node_t> topological_nodes() const;
  inline const complement_property_map_t& complement() { return _complement; }

  /* cover */
  bool has_cover() const;
  const xmg_cover& cover() const;
  void set_cover( const xmg_cover& other );

  /* ref counting */
  void init_refs();
  unsigned get_ref( xmg_node n ) const;
  unsigned inc_ref( xmg_node n );
  unsigned dec_ref( xmg_node n );
  void inc_output_refs();

  /* marking */
  xmg_bitmarks& bitmarks();
  const xmg_bitmarks& bitmarks() const;

  void mark_as_modified();

public: /* properties */
  inline void set_native_xor( bool native_xor ) { _native_xor = native_xor; }
  inline bool has_native_xor() const            { return _native_xor; }

  inline void set_structural_hashing( bool enabled ) { _enable_structural_hashing = enabled; }
  inline bool has_structural_hashing() const         { return _enable_structural_hashing; }

  inline void set_inverter_propagation( bool enabled ) { _enable_inverter_propagation = enabled; }
  inline bool has_inverter_propagation() const         { return _enable_inverter_propagation; }

private:
  graph_t g;
  node_t  constant;

  std::string  _name;
  input_vec_t  _inputs;
  output_vec_t _outputs;
  std::unordered_map<xmg_node, unsigned> _input_to_id;

  using maj_strash_key_t = std::tuple<xmg_function, xmg_function, xmg_function>;
  using xor_strash_key_t = std::pair<xmg_function, xmg_function>;
  std::unordered_map<maj_strash_key_t, node_t, hash<maj_strash_key_t>> maj_strash;
  std::unordered_map<xor_strash_key_t, node_t, hash<xor_strash_key_t>> xor_strash;

  complement_property_map_t               _complement;

  /* additional network information */
  dirty<std::vector<unsigned>>            fanout;
  dirty<std::vector<std::vector<node_t>>> parentss;
  dirty<std::vector<unsigned>>            levels;

  /* network settings and stats */
  bool                                    _native_xor = true;
  bool                                    _enable_structural_hashing = true;
  bool                                    _enable_inverter_propagation = true;
  unsigned                                _num_maj = 0u;
  unsigned                                _num_xor = 0u;

  /* cover */
  std::shared_ptr<xmg_cover>              _cover = nullptr;

  /* bitmarks */
  std::shared_ptr<xmg_bitmarks>           _bitmarks;

  /* utilities */
  std::vector<unsigned>                   ref_count;
};

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
