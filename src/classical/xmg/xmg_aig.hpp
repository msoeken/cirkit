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
 * @file xmg_aig.hpp
 *
 * @brief Convert XMGs to AIGs and vice versa
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef XMG_AIG_HPP
#define XMG_AIG_HPP

#include <classical/aig.hpp>
#include <classical/xmg/xmg.hpp>

namespace cirkit
{

aig_graph create_aig_topological( const xmg_graph& circ );
aig_graph create_aig_top_down( const xmg_graph& circ, const xmg_function& f );

xmg_graph xmg_from_aig( const aig_graph& aig );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
