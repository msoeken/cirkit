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

#include "terminal.hpp"

#include <iomanip>
#include <iostream>

namespace cirkit
{

void print_banner( const std::string& caption, unsigned width )
{
  std::cout << '+' << std::string( width - 2u, '-' ) << '+' << std::endl
            << "| " << std::left << std::setw( width - 3u ) << caption << '|' << std::endl
            << '+' << std::string( width - 2u, '-' ) << '+' << std::endl;
}

progress_line::progress_line( const std::string& format, bool enable, std::ostream& os )
  : format( format ),
    enable( enable ),
    os( os )
{
}

progress_line::~progress_line()
{
  if ( !enable ) return;
  clear();

  if ( _keep_last )
  {
    os << last_string << std::endl;
  }
}

void progress_line::clear()
{
  if ( !enable ) return;
  os << std::string( max_length, ' ' ) << '\r';
  os.flush();
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
