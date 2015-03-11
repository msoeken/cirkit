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
 * @file optimization.hpp
 *
 * @brief General classical optimization type definitions
 *
 * @author Mathias Soeken
 * @since  2.0
 */

#ifndef CLASSICAL_OPTIMIZATION_HPP
#define CLASSICAL_OPTIMIZATION_HPP

#include <string>

#include <boost/dynamic_bitset.hpp>
#include <boost/function.hpp>

#include <cudd.h>

#include <core/functor.hpp>

namespace cirkit
{

  using cube_t          = std::pair<boost::dynamic_bitset<>, boost::dynamic_bitset<>>;
  using cube_function_t = std::function<void(const cube_t&)>;


  using dd_based_esop_optimization_func  = functor<void( DdManager*, DdNode* )>;
  using pla_based_esop_optimization_func = functor<void( const std::string& )>;

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
