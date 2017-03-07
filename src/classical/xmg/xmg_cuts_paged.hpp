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
 * @file xmg_cuts_paged.hpp
 *
 * @brief XMG cut enumeration
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef XMG_CUTS_PAGED_HPP
#define XMG_CUTS_PAGED_HPP

#include <map>
#include <vector>

#include <boost/dynamic_bitset.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/range/iterator_range.hpp>

#include <core/properties.hpp>
#include <core/utils/paged_memory.hpp>
#include <classical/xmg/xmg.hpp>
#include <classical/xmg/xmg_xor_blocks.hpp>
#include <classical/utils/truth_table_utils.hpp>

namespace cirkit
{


std::vector<std::pair<unsigned, unsigned>> compute_level_ranges( xmg_graph& xmg, unsigned& max_level );

class xmg_cuts_paged final
{
public:
  using cut = paged_memory::set;
  using cone = paged_memory::set;

  xmg_cuts_paged( xmg_graph& xmg, unsigned k, const properties::ptr& settings = properties::ptr() );
  xmg_cuts_paged( xmg_graph& xmg, unsigned k,
                  const std::vector<xmg_node>& start,
                  const std::vector<xmg_node>& boundary,
                  const std::vector<std::pair<unsigned, unsigned>>& levels,
                  const properties::ptr& settings = properties::ptr() );

  const xmg_graph& xmg() const;

  unsigned total_cut_count() const;
  double enumeration_time() const;

  unsigned memory() const;
  unsigned count( xmg_node node ) const;
  boost::iterator_range<paged_memory::iterator> cuts( xmg_node node );
  boost::iterator_range<paged_memory::iterator> cut_cones( xmg_node node );

  tt simulate( xmg_node node, const cut& c ) const;
  unsigned depth( xmg_node node, const cut& c ) const;
  unsigned size( xmg_node node, const cut& c ) const;

  unsigned index( const cut& c ) const;
  cut      from_address( unsigned address );

  void foreach_cut( const std::function<void(xmg_node, cut&)>& func );

private:
  void enumerate();
  void enumerate_with_xor_blocks( const std::unordered_map<xmg_node, xmg_xor_block_t>& blocks );
  void enumerate_partial( const std::vector<xmg_node>& start, const std::vector<xmg_node>& boundary );

  void enumerate_node_with_bitsets( xmg_node n, const std::vector<xmg_node>& ns );

  using local_cut_vec_t = std::vector<std::tuple<boost::dynamic_bitset<>, unsigned, boost::dynamic_bitset<>>>;
  local_cut_vec_t enumerate_local_cuts( xmg_node n1, xmg_node n2, unsigned max_cut_size );
  local_cut_vec_t enumerate_local_cuts( xmg_node n1, xmg_node n2, xmg_node n3, unsigned max_cut_size );
  local_cut_vec_t enumerate_local_cuts( const std::vector<xmg_node>& ns, unsigned max_cut_size );
  void merge_cut( local_cut_vec_t& local_cuts, const boost::dynamic_bitset<>& new_cut, unsigned min_level, const boost::dynamic_bitset<>& new_cone ) const;

  std::vector<unsigned> get_extra( unsigned depth, unsigned size ) const;

private:
  const xmg_graph& _xmg;
  unsigned         _k;
  unsigned         _priority = 8u;
  unsigned         _extra    = 0u;
  bool             _progress = false;
  paged_memory     data;
  paged_memory     cones;

  double           _enumeration_time = 0.0;

  unsigned         _top_index = 0u; /* index when doing topo traversal */

  std::vector<std::pair<unsigned, unsigned>> _levels;
};

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
