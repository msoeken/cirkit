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
 * @file size.hpp
 *
 * @brief Count number of nodes
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef SIZE_HPP
#define SIZE_HPP

#include <vector>

#include <classical/dd/dd_depth_first.hpp>

namespace cirkit
{

template<class node>
unsigned long dd_size( const node& f )
{
  auto size = 0ul;
  dd_depth_first<node>( f, [&]( const node& n ) { ++size; } );
  return size;
}

template<class node>
unsigned long dd_size( const std::vector<node>& fs )
{
  auto size = 0ul;
  dd_depth_first<node>( fs, [&]( const node& n ) { ++size; } );
  return size;
}

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
