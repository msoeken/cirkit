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

#include "extract_pla.hpp"

#include <fstream>

#include <core/io/pla_parser.hpp>
#include <core/utils/range_utils.hpp>

namespace cirkit
{

class extract_pla_processor : public pla_processor
{
public:
  extract_pla_processor( std::ostream& os, unsigned output )
    : os( os ),
      output( output ) {}

  void on_num_inputs( unsigned num_inputs )
  {
    os << ".i " << num_inputs << std::endl
       << ".o 1" << std::endl;
  }

  void on_input_labels( const std::vector<std::string>& input_labels )
  {
    os << ".ilb " << any_join( input_labels, " " ) << std::endl;
  }

  void on_output_labels( const std::vector<std::string>& output_labels )
  {
    os << ".ob " << output_labels[output] << std::endl;
  }

  void on_end()
  {
    os << ".e" << std::endl;
  }

  void on_cube( const std::string& in, const std::string& out )
  {
    if ( out[output] == '1' )
    {
      os << in << " 1" << std::endl;
    }
  }

  std::ostream& os;
  unsigned output;
};

void extract_pla( const std::string& filename, const std::string& destination, unsigned output )
{
  std::ofstream os( destination.c_str(), std::ofstream::out );
  extract_pla_processor processor( os, output );
  pla_parser( filename, processor );
  os.close();
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
