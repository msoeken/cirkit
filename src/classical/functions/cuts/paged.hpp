/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2016  EPFL
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
  boost::iterator_range<paged_memory::iterator> cuts( aig_node node ) const;

  tt simulate( aig_node node, const cut& c ) const;
  unsigned depth( aig_node node, const cut& c ) const;

private:
  void enumerate();
  void enumerate_node_with_bitsets( aig_node n, aig_node n1, aig_node n2 );
  std::vector<std::pair<boost::dynamic_bitset<>, unsigned>> enumerate_local_cuts( aig_node n1, aig_node n2, unsigned max_cut_size ) const;

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
