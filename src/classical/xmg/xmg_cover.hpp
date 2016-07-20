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
 * @file xmg_cover.hpp
 *
 * @brief Store XMG covers
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef XMG_COVER_HPP
#define XMG_COVER_HPP

#include <string>
#include <vector>

#include <boost/range/iterator_range.hpp>

#include <classical/xmg/xmg.hpp>
#include <classical/xmg/xmg_cuts_paged.hpp>

namespace cirkit
{

class xmg_cover
{
public:
  using index_range = boost::iterator_range<std::vector<unsigned>::const_iterator>;

  xmg_cover( unsigned cut_size, const xmg_graph& xmg );

  void add_cut( xmg_node n, const xmg_cuts_paged::cut& cut );
  bool has_cut( xmg_node n ) const;
  index_range cut( xmg_node n ) const;

  inline unsigned cut_size() const { return _cut_size; }
  inline unsigned lut_count() const { return count; }

private:
  unsigned              _cut_size; /* remember cut_size */

  std::vector<unsigned> offset; /* address from node index to leafs, 0 if unused */
  std::vector<unsigned> leafs;  /* first element is unused, then | #leafs | l_1 | l_2 | ... | l_k | */
  unsigned              count = 0u;
};

void xmg_cover_write_dot( const xmg_graph& xmg, const std::string& filename );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
