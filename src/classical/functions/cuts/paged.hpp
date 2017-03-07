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
 * @file paged.hpp
 *
 * @brief Cut enumeration based on paged memory
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef CUTS_PAGED_HPP
#define CUTS_PAGED_HPP

#include <map>
#include <vector>

#include <boost/dynamic_bitset.hpp>
#include <boost/range/iterator_range.hpp>

#include <core/utils/paged_memory.hpp>
#include <classical/aig.hpp>
#include <classical/utils/truth_table_utils.hpp>

namespace cirkit
{

class paged_aig_cuts final
{
public:
  using cut = paged_memory::set;

  paged_aig_cuts( const aig_graph& aig, unsigned k, bool parallel = true, unsigned priority = 8u );

  unsigned total_cut_count() const;
  double enumeration_time() const;

  unsigned memory() const;
  unsigned count( aig_node node ) const;
  boost::iterator_range<paged_memory::iterator> cuts( aig_node node );

  tt simulate( aig_node node, const cut& c ) const;
  unsigned depth( aig_node node, const cut& c ) const;

private:
  void enumerate();
  void enumerate_node_with_bitsets( aig_node n, aig_node n1, aig_node n2 );
  std::vector<std::pair<boost::dynamic_bitset<>, unsigned>> enumerate_local_cuts( aig_node n1, aig_node n2, unsigned max_cut_size );

  void enumerate_parallel();

private:
  const aig_graph&             _aig;
  unsigned                     _k;
  unsigned                     _priority = 8u;
  paged_memory                 data;

  double                       _enumeration_time = 0.0;

  unsigned                     _top_index = 0u; /* index when doing topo traversal */

  std::map<aig_node, unsigned> _levels;
};

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
