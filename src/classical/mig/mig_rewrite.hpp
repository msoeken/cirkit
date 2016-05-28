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
