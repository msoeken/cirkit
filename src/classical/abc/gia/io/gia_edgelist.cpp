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

#include "gia_edgelist.hpp"

#include <fstream>

#include <fmt/format.h>

namespace cirkit
{

void gia_write_lut_cover_to_edgelist( const gia_graph& gia, const std::string& filename )
{
  std::ofstream os( filename.c_str(), std::ofstream::out );
  gia_write_lut_cover_to_edgelist( gia, os );
}

void gia_write_lut_cover_to_edgelist( const gia_graph& gia, std::ostream& os )
{
  /* connect LUTs */
  gia.foreach_lut([&gia, &os]( int id ) {
      gia.foreach_lut_fanin(id, [id, &gia, &os]( int fanin ) {
          if ( !abc::Gia_ObjIsCi( gia.object( fanin ) ) )
          {
            os << fmt::format( "{} {}\n", id, fanin );
          }
        });
    });

  os << std::flush;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
