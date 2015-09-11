/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
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
