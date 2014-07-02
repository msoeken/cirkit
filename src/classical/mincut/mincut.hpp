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
 * @file mincut.hpp
 *
 * @brief Min-Cut algorithms in AIGs
 *
 * @author Mathias Soeken
 * @since  2.0
 */

#ifndef MINCUT_HPP
#define MINCUT_HPP

#include <list>

#include <core/functor.hpp>

#include <classical/aig.hpp>

namespace cirkit
{

  typedef functor<bool(std::list<std::list<aig_function>>&, aig_graph&, unsigned count)> mincut_by_edge_func;
  typedef functor<bool(std::list<std::list<aig_node>>&, aig_graph&, unsigned count)> mincut_by_node_func;

}

#endif

// Local Variables:
// c-basic-offset: 2
// End:
