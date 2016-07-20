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
 * @file xmg_expr.hpp
 *
 * @brief XMG from and to expressions
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef XMG_EXPR_HPP
#define XMG_EXPR_HPP

#include <vector>

#include <classical/utils/expression_parser.hpp>
#include <classical/xmg/xmg.hpp>

namespace cirkit
{

expression_t::ptr xmg_to_expression( const xmg_graph& xmg, const xmg_function& f );
xmg_function xmg_from_expression( xmg_graph& xmg, std::vector<xmg_function>& pis, const expression_t::ptr& expr );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
