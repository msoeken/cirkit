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
 * @file mig_from_string.hpp
 *
 * @brief Creates an MIG from a string
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef MIG_FROM_STRING_HPP
#define MIG_FROM_STRING_HPP

#include <string>
#include <vector>

#include <core/properties.hpp>
#include <classical/mig/mig.hpp>
#include <classical/utils/expression_parser.hpp>

namespace cirkit
{

mig_graph mig_from_string( const std::string& expr,
                           const properties::ptr& settings = properties::ptr(),
                           const properties::ptr& statistics = properties::ptr() );

mig_function mig_from_string( mig_graph& mig, const std::string& expr,
                              const properties::ptr& settings = properties::ptr(),
                              const properties::ptr& statistics = properties::ptr() );

std::string mig_to_string( const mig_graph& mig, const mig_function& f,
                           const properties::ptr& settings = properties::ptr(),
                           const properties::ptr& statistics = properties::ptr() );

expression_t::ptr mig_to_expression( const mig_graph& mig, const mig_function& f );
mig_function mig_from_expression( mig_graph& mig, std::vector<mig_function>& pis, const expression_t::ptr& expr );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
