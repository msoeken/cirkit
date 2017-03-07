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
 * @file paged_mig.hpp
 *
 * @brief MIG cut enumeration based on paged memory
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef CUTS_PAGED_MIG_HPP
#define CUTS_PAGED_MIG_HPP

#include <map>
#include <vector>

#include <boost/dynamic_bitset.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/range/iterator_range.hpp>

#include <core/utils/paged_memory.hpp>
#include <classical/mig/mig.hpp>
#include <classical/utils/truth_table_utils.hpp>

namespace cirkit
{

std::vector<std::pair<unsigned, unsigned>> compute_level_ranges( const mig_graph& mig, unsigned& max_level );

class mig_cuts_paged final
{
public:
  using cut = paged_memory::set;
  using cone = paged_memory::set;

  mig_cuts_paged( const mig_graph& mig, unsigned k, unsigned priority = 8u );
  mig_cuts_paged( const mig_graph& mig, unsigned k,
                  const std::vector<mig_node>& start,
                  const std::vector<mig_node>& boundary,
                  const std::vector<std::pair<unsigned, unsigned>>& levels,
                  unsigned priority = 8u );

  unsigned total_cut_count() const;
  double enumeration_time() const;

  unsigned memory() const;
  unsigned count( mig_node node ) const;
  boost::iterator_range<paged_memory::iterator> cuts( mig_node node );
  boost::iterator_range<paged_memory::iterator> cut_cones( mig_node node );

  tt simulate( mig_node node, const cut& c ) const;
  unsigned depth( mig_node node, const cut& c ) const;
  unsigned size( mig_node node, const cut& c ) const;

private:
  void enumerate();
  void enumerate_partial( const std::vector<mig_node>& start, const std::vector<mig_node>& boundary );
  void enumerate_node_with_bitsets( mig_node n, mig_node n1, mig_node n2, mig_node n3 );
  std::vector<std::tuple<boost::dynamic_bitset<>, unsigned, boost::dynamic_bitset<>>> enumerate_local_cuts( mig_node n1, mig_node n2, mig_node n3, unsigned max_cut_size );

private:
  const mig_graph& _mig;
  unsigned         _k;
  unsigned         _priority = 8u;
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
