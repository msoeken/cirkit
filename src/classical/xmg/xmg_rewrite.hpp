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
#include <unordered_map>

#include <boost/optional.hpp>

#include <core/properties.hpp>
#include <classical/xmg/xmg.hpp>

namespace cirkit
{

using maj_rewrite_func_t    = std::function<xmg_function(xmg_graph&, const xmg_function&, const xmg_function&, const xmg_function&)>;
using xor_rewrite_func_t    = std::function<xmg_function(xmg_graph&, const xmg_function&, const xmg_function&)>;
using prefill_func_t        = std::function<void(xmg_graph&, std::map<xmg_node, xmg_function>&)>;
using xmg_init_func_t       = std::function<void(xmg_graph&)>;
using xmg_substitutes_map_t = boost::optional<std::unordered_map<xmg_node, xmg_function>>;

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
xmg_graph xmg_merge( const xmg_graph& xmg1, const xmg_graph& xmg2, const properties::ptr& settings = properties::ptr(), const properties::ptr& statistics = properties::ptr() );
xmg_graph xmg_to_mig( const xmg_graph& xmg, const properties::ptr& settings = properties::ptr(), const properties::ptr& statistics = properties::ptr() );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
