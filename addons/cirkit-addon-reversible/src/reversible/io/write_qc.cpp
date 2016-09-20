/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2016  EPFL
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

#include "write_qc.hpp"

#include <fstream>

#include <boost/algorithm/string/join.hpp>

#include <core/utils/range_utils.hpp>
#include <reversible/target_tags.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

void write_qc( const circuit& circ, std::ostream& os )
{
  const auto vars = create_name_list( "v%d", circ.lines() );

  os << ".v " << boost::join( vars, " " ) << std::endl
     << "BEGIN" << std::endl;

  for ( const auto& gate : circ )
  {
    assert( is_toffoli( gate ) );

    os << "t" << ( gate.controls().size() + 1 );
    for ( const auto& c : gate.controls() )
    {
      os << " " << vars[c.line()];
      if ( !c.polarity() )
      {
        os << "'";
      }
    }
    os << " " << vars[gate.targets().front()] << std::endl;
  }

  os << "END" << std::endl;
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

void write_qc( const circuit& circ, const std::string& filename )
{
  std::ofstream os( filename.c_str(), std::ofstream::out );
  write_qc( circ, os );
  os.close();
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
