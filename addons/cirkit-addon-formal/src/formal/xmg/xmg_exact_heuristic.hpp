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
 * @file xmg_exact_heuristic.hpp
 *
 * @brief Heuristic based on exact XMG
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef XMG_EXACT_HEURISTIC_HPP
#define XMG_EXACT_HEURISTIC_HPP

#include <core/properties.hpp>
#include <classical/utils/truth_table_utils.hpp>
#include <classical/xmg/xmg.hpp>

namespace cirkit
{

xmg_graph xmg_exact_heuristic( const tt& spec, const properties::ptr& settings = properties::ptr(), const properties::ptr& statistics = properties::ptr() );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
