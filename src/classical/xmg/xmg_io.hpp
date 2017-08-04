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
xmg_graph xmg_read_yig( const std::string& filename );

void write_bench( const xmg_graph& xmg, const std::string& filename );

void write_verilog( const xmg_graph& xmg, const std::string& filename, const properties::ptr& settings = properties::ptr() );
void write_verilog( const xmg_graph& xmg, std::ostream& os, const properties::ptr& settings = properties::ptr() );

void write_smtlib2( const xmg_graph& xmg, const std::string& filename, const properties::ptr& settings = properties::ptr() );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
