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

#include "pla_parser.hpp"

#include "pla_processor.hpp"

#include <fstream>
#include <regex>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/range/algorithm.hpp>

namespace cirkit
{

bool pla_parser( std::istream& in, pla_processor& reader, bool skip_after_first_cube )
{
  std::string line;
  std::regex whitespace( "\\s+" );

  while ( in.good() && getline( in, line ) )
  {
    boost::trim( line );
    line = std::regex_replace( line, whitespace, std::string( " " ) );
    if ( !line.size() ) { continue; }

    if ( boost::starts_with( line, "#" ) )
    {
      std::string comment( boost::find_if( line, []( char c ) { return c != '#'; } ), line.end() );
      reader.on_comment( comment );
    }
    else if ( boost::starts_with( line, ".i " ) )
    {
      reader.on_num_inputs( boost::lexical_cast<unsigned>( line.substr( 3 ) ) );
    }
    else if ( boost::starts_with( line, ".o " ) )
    {
      reader.on_num_outputs( boost::lexical_cast<unsigned>( line.substr( 3 ) ) );
    }
    else if ( boost::starts_with( line, ".p " ) )
    {
      reader.on_num_products( boost::lexical_cast<unsigned>( line.substr( 3 ) ) );
    }
    else if ( boost::starts_with( line, ".ilb " ) )
    {
      std::vector<std::string> input_labels;
      std::string rem = line.substr( 5 );
      boost::split( input_labels, rem, boost::is_space(), boost::token_compress_on );
      reader.on_input_labels( input_labels );
    }
    else if ( boost::starts_with( line, ".ob " ) )
    {
      std::vector<std::string> output_labels;
      std::string rem = line.substr( 4 );
      boost::split( output_labels, rem, boost::is_space(), boost::token_compress_on );
      reader.on_output_labels( output_labels );
    }
    else if ( line == ".e" )
    {
      reader.on_end();
    }
    else if ( boost::starts_with( line, ".type " ) )
    {
      reader.on_type( line.substr( 6 ) );
    }
    else
    {
      assert( line.size() && ( line[0] == '0' || line[0] == '1' || line[0] == '-' ) );

      std::vector<std::string> inout;
      boost::split( inout, line, boost::is_any_of(" \t|" ), boost::token_compress_on );
      assert( inout.size() == 2 );
      reader.on_cube( inout[0], inout[1] );

      if ( skip_after_first_cube )
      {
        break;
      }
    }
  }

  return true;
}

bool pla_parser( const std::string& filename, pla_processor& reader, bool skip_after_first_cube )
{
  std::ifstream is( filename.c_str(), std::ifstream::in );
  auto r = pla_parser( is, reader, skip_after_first_cube );
  is.close();
  return r;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
