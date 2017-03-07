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
