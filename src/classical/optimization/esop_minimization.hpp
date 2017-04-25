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
 * @file esop_minimization.hpp
 *
 * @brief ESOP minimization
 *
 * @author Mathias Soeken
 * @since  2.0
 */

#ifndef ESOP_MINIMIZATION_HPP
#define ESOP_MINIMIZATION_HPP

#include <functional>
#include <map>
#include <string>

#include <boost/dynamic_bitset.hpp>

#include <cudd.h>

#include <core/properties.hpp>
#include <classical/optimization/optimization.hpp>

namespace cirkit
{

/**
 * For PSDKRO generation
 */

typedef std::pair<unsigned, unsigned> exp_cost_t;
typedef std::map<DdNode*, exp_cost_t> exp_cache_t;

exp_cost_t count_cubes_in_exact_psdkro( DdManager * cudd, DdNode * f, exp_cache_t& exp_cache );
void generate_exact_psdkro( DdManager * cudd, DdNode * f, char * var_values, int last_index, const exp_cache_t& exp_cache, const std::function<void()>& on_cube );

/**
 * @brief ESOP minimization
 *
 * This convenience function takes the BDD node that contains
 * the function to be optimized directly.
 *
 * @author Mathias Soeken
 */
void esop_minimization( DdManager * cudd, DdNode * f,
                        properties::ptr settings = properties::ptr(),
                        properties::ptr statistics = properties::ptr() );

/**
 * @brief ESOP minimization
 *
 * This optimization algorithm is an implementation of the paper
 * [A. Mishchenko and M. Perkowski, Reed Muller Workshop 5 (2001)].
 * The paper has also been implemented in the tool EXORCISM-4 by the
 * same authors.
 *
 * In comparison to EXORCISM-4 this algorithm does not support functions
 * with mulitple outputs.
 *
 * @author Mathias Soeken
 */
void esop_minimization( const std::string& filename,
                        properties::ptr settings = properties::ptr(),
                        properties::ptr statistics = properties::ptr() );

dd_based_esop_optimization_func dd_based_esop_minimization_func( properties::ptr settings = std::make_shared<properties>(),
                                                                 properties::ptr statistics = std::make_shared<properties>() );

pla_based_esop_optimization_func pla_based_esop_minimization_func( properties::ptr settings = std::make_shared<properties>(),
                                                                   properties::ptr statistics = std::make_shared<properties>() );

void test_change_performance();

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
