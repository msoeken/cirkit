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

#include "read_verilog.hpp"

#include <classical/abc/abc_api.hpp>
#include <classical/abc/functions/gia_to_cirkit.hpp>

#include <base/wlc/wlc.h>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

aig_graph read_verilog_with_abc( const std::string& filename, const properties::ptr& settings, const properties::ptr& statistics )
{
  auto * wlc = abc::Wlc_ReadVer( const_cast<char*>( filename.c_str() ), nullptr );
  auto * gia = abc::Wlc_NtkBitBlast( wlc, nullptr, -1, 2, 0, 0, 0 );
  return gia_to_cirkit( gia );
}

aig_graph read_verilog_string_with_abc( const std::string& str, const properties::ptr& settings, const properties::ptr& statistics )
{
  auto * wlc = abc::Wlc_ReadVer( nullptr, const_cast<char*>( str.c_str() ) );
  auto * gia = abc::Wlc_NtkBitBlast( wlc, nullptr, -1, 2, 0, 0, 0 );
  return gia_to_cirkit( gia );
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
