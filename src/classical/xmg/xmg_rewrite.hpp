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
 * @file xmg_rewrite.hpp
 *
 * @brief Generic rewriting on XMGs
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef XMG_REWRITE_HPP
#define XMG_REWRITE_HPP

#include <functional>
#include <map>

#include <core/properties.hpp>
#include <classical/xmg/xmg.hpp>

namespace cirkit
{

using maj_rewrite_func_t = std::function<xmg_function(xmg_graph&, const xmg_function&, const xmg_function&, const xmg_function&)>;
using xor_rewrite_func_t = std::function<xmg_function(xmg_graph&, const xmg_function&, const xmg_function&)>;
using prefill_func_t     = std::function<void(xmg_graph&, std::map<xmg_node, xmg_function>&)>;
using xmg_init_func_t    = std::function<void(xmg_graph&)>;

xmg_function rewrite_default_maj( xmg_graph& xmg_new, const xmg_function& a, const xmg_function& b, const xmg_function& c );
xmg_function rewrite_default_xor( xmg_graph& xmg_new, const xmg_function& a, const xmg_function& b );

xmg_graph xmg_rewrite_top_down( const xmg_graph& xmg,
                                const maj_rewrite_func_t& on_maj,
                                const xor_rewrite_func_t& on_xor,
                                const properties::ptr& settings = properties::ptr(),
                                const properties::ptr& statistics = properties::ptr() );

std::vector<xmg_function> xmg_rewrite_top_down_inplace( xmg_graph& dest,
                                                        const xmg_graph& xmg,
                                                        const maj_rewrite_func_t& on_maj,
                                                        const xor_rewrite_func_t& on_xor,
                                                        const std::vector<xmg_function>& pi_mapping,
                                                        const properties::ptr& settings = properties::ptr(),
                                                        const properties::ptr& statistics = properties::ptr() );

xmg_graph xmg_rewrite_bottom_up( const xmg_graph& xmg,
                                 const maj_rewrite_func_t& on_maj,
                                 const xor_rewrite_func_t& on_xor,
                                 const properties::ptr& settings = properties::ptr(),
                                 const properties::ptr& statistics = properties::ptr() );

xmg_graph xmg_strash( const xmg_graph& xmg, const properties::ptr& settings = properties::ptr(), const properties::ptr& statistics = properties::ptr() );
xmg_graph xmg_to_mig( const xmg_graph& xmg, const properties::ptr& settings = properties::ptr(), const properties::ptr& statistics = properties::ptr() );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
