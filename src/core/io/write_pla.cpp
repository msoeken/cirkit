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

#include "write_pla.hpp"

#include <fstream>

#include <core/utils/range_utils.hpp>

namespace cirkit
{

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

void write_pla( const bdd_function_t& bdd, const std::string& filename )
{
  std::ofstream os( filename.c_str(), std::ofstream::out );
  write_pla( bdd, os );
  os.close();
}

void write_pla( const bdd_function_t& bdd, std::ostream& os )
{
  DdGen * gen;
  int * ddcube;
  CUDD_VALUE_TYPE value;

  os << ".i " << bdd.first.ReadSize() << std::endl
     << ".o " << bdd.second.size() << std::endl;

  for ( const auto& f : index( bdd.second ) )
  {
    std::string output( bdd.second.size(), '0' );
    output[f.index] = '1';

    Cudd_ForeachCube( bdd.first.getManager(), f.value.getNode(), gen, ddcube, value )
    {

      std::string input( bdd.first.ReadSize(), '-' );

      for ( auto i = 0; i < bdd.first.ReadSize(); ++i )
      {
        switch ( ddcube[i] )
        {
        case 0:
          input[i] = '0';
          break;
        case 1:
          input[i] = '1';
          break;
        default:
          break;
        }
      }

      os << input << " " << output << std::endl;
    }
  }

  os << ".e" << std::endl;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
