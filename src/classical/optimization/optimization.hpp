/* RevKit: A Toolkit for Reversible Circuit Design (www.revkit.org)
 * Copyright (C) 2009-2013  The RevKit Developers <revkit@informatik.uni-bremen.de>
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

#include <core/circuit.hpp>
#include <core/functor.hpp>
#include <core/truth_table.hpp>

namespace revkit
{

  typedef std::pair<boost::dynamic_bitset<>, boost::dynamic_bitset<> > cube_t;
  typedef std::function<void(const cube_t&)> cube_function_t;


  typedef functor<void( DdManager*, DdNode* )> dd_based_esop_optimization_func;
  typedef functor<void( const std::string& )> pla_based_esop_optimization_func;

}

#endif

// Local Variables:
// c-basic-offset: 2
// End:
