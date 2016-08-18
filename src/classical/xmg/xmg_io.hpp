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
 * @file xmg_io.hpp
 *
 * @brief I/O routines for XMG
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef XMG_IO_HPP
#define XMG_IO_HPP

#include <string>

#include <core/properties.hpp>
#include <classical/xmg/xmg.hpp>

namespace cirkit
{

xmg_graph read_verilog( const std::string& filename, bool native_xor = true, bool enable_structural_hashing = true, bool enable_inverter_propagation = true );

void write_bench( const xmg_graph& xmg, const std::string& filename );
void write_verilog( const xmg_graph& xmg, const std::string& filename, const properties::ptr& settings = properties::ptr() );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
