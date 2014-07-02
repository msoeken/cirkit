/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2014  University of Bremen
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
 * @file find_mincut_blocking.hpp
 *
 * @brief Find Min-Cut in AIG graph based on nodes and blocking
 *
 * @author Mathias Soeken
 * @since  2.0
 */

#ifndef FIND_MINCUT_BLOCKING_HPP
#define FIND_MINCUT_BLOCKING_HPP

#include <classical/mincut/mincut.hpp>

namespace cirkit
{

  /**
   * @param cuts   List of cuts, where a cut is a list of AIG nodes
   * @param aig    AIG graph (will not be changed, instead a copy is created)
   * @param count  Number of cuts that should be determined
   */
  bool find_mincut_blocking( std::list<std::list<aig_node>>& cuts, aig_graph& aig, unsigned count,
                             properties::ptr settings = properties::ptr(), properties::ptr statistics = properties::ptr() );

  mincut_by_node_func find_mincut_blocking_func( properties::ptr settings = properties::ptr( new properties ),
                                                 properties::ptr statistics = properties::ptr( new properties ) );

}

#endif

// Local Variables:
// c-basic-offset: 2
// End:
