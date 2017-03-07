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
 * @file unate.hpp
 *
 * @brief Check for unateness
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef UNATE_HPP
#define UNATE_HPP

#include <vector>

#include <boost/dynamic_bitset.hpp>

#include <core/properties.hpp>
#include <classical/aig.hpp>

namespace cirkit
{

/**
 * The return value contains all the information for each output/input pair
 * ordered first by outputs, then by inputs, each pair takes 2 bits in the bitset:
 *
 * 00: binate
 * 01: unate_pos
 * 10: unate_neg
 * 11: independent
 */
boost::dynamic_bitset<> unateness_naive( const aig_graph& aig,
                                         const properties::ptr& settings = properties::ptr(),
                                         const properties::ptr& statistics = properties::ptr() );

boost::dynamic_bitset<> unateness_split( const aig_graph& aig,
                                         const properties::ptr& settings = properties::ptr(),
                                         const properties::ptr& statistics = properties::ptr() );

boost::dynamic_bitset<> unateness_split_parallel( const aig_graph& aig,
                                                  const properties::ptr& settings = properties::ptr(),
                                                  const properties::ptr& statistics = properties::ptr() );

boost::dynamic_bitset<> unateness_split_inputs_parallel( const aig_graph& aig,
                                                         const properties::ptr& settings = properties::ptr(),
                                                         const properties::ptr& statistics = properties::ptr() );

boost::dynamic_bitset<> unateness( const aig_graph& aig,
                                   const properties::ptr& settings = properties::ptr(),
                                   const properties::ptr& statistics = properties::ptr() );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
