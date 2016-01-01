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
