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
 * @file error_metrics.hpp
 *
 * @brief Error metrics for approximate computing
 *
 * @author Mathias Soeken
 * @author Arun Chandrasekharan
 * @since  2.3
 */

#ifndef ERROR_METRICS_HPP
#define ERROR_METRICS_HPP

#include <vector>

#include <boost/multiprecision/cpp_dec_float.hpp>
#include <boost/multiprecision/cpp_int.hpp>

#include <core/properties.hpp>
#include <classical/dd/bdd.hpp>

namespace cirkit
{

enum class worst_case_maximum_method { shift, chi };

boost::multiprecision::uint256_t error_rate( const std::vector<bdd>& f, const std::vector<bdd>& fhat,
                                             const properties::ptr& settings = properties::ptr(),
                                             const properties::ptr& statistics = properties::ptr() );
boost::multiprecision::uint256_t worst_case( const std::vector<bdd>& f, const std::vector<bdd>& fhat,
                                             const properties::ptr& settings = properties::ptr(),
                                             const properties::ptr& statistics = properties::ptr() );
boost::multiprecision::cpp_dec_float_100 average_case( const std::vector<bdd>& f, const std::vector<bdd>& fhat,
                                                       const properties::ptr& settings = properties::ptr(),
                                                       const properties::ptr& statistics = properties::ptr() );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
