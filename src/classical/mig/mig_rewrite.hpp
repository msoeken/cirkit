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
 * @file mig_rewrite.hpp
 *
 * @brief Generic rewriting on MIGs
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef MIG_REWRITE_HPP
#define MIG_REWRITE_HPP

#include <functional>
#include <map>

#include <core/properties.hpp>
#include <classical/mig/mig.hpp>

namespace cirkit
{

using mig_maj_rewrite_func_t = std::function<mig_function(mig_graph&, const mig_function&, const mig_function&, const mig_function&)>;
using prefill_func_t         = std::function<void(mig_graph&, std::map<mig_node, mig_function>&)>;

mig_function mig_rewrite_default_maj( mig_graph& mig_new, const mig_function& a, const mig_function& b, const mig_function& c );

mig_graph mig_rewrite_top_down( const mig_graph& mig,
                                const mig_maj_rewrite_func_t& on_maj,
                                const properties::ptr& settings = properties::ptr(),
                                const properties::ptr& statistics = properties::ptr() );

mig_graph mig_rewrite_bottom_up( const mig_graph& mig,
                                 const mig_maj_rewrite_func_t& on_maj,
                                 const properties::ptr& settings = properties::ptr(),
                                 const properties::ptr& statistics = properties::ptr() );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
