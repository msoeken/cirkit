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
 * @file dd_depth_first.hpp
 *
 * @brief Depth-first search traversal in DD
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef DD_DEPTH_FIRST_HPP
#define DD_DEPTH_FIRST_HPP

#include <functional>
#include <vector>

#include <boost/assign/std/vector.hpp>
#include <boost/range/algorithm.hpp>

using namespace boost::assign;

namespace cirkit
{

namespace detail
{

template<class node>
using node_func_t = std::function<void(const node&)>;

template<class node>
void dd_depth_first_rec( const node& n, std::vector<unsigned>& visited, const node_func_t<node>& f )
{
  if ( n.index <= 1 ) {
    return;
  }
  if ( boost::find( visited, n.index ) != visited.end() ) { return; }
  visited += n.index;

  auto l = n.low(); auto h = n.high();
  if ( l.index > 1 && boost::find( visited, l.index ) == visited.end() ) { dd_depth_first_rec( l, visited, f ); }
  if ( h.index > 1 && boost::find( visited, h.index ) == visited.end() ) { dd_depth_first_rec( h, visited, f ); }

  f( n );
}

}

template<class node>
void dd_depth_first( const node& n, const detail::node_func_t<node>& f )
{
  std::vector<unsigned> visited;
  detail::dd_depth_first_rec( n, visited, f );
}

template<class node>
void dd_depth_first( const std::vector<node>& ns, const detail::node_func_t<node>& f )
{
  std::vector<unsigned> visited;
  for ( const auto& n : ns )
  {
    detail::dd_depth_first_rec( n, visited, f );
  }
}

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
