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
