/* RevKit: A Toolkit for Reversible Circuit Design (www.revkit.org)
 * Copyright (C) 2009-2013  The RevKit Developers <revkit@informatik.uni-bremen.de>
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

#include "pla_parser.hpp"

#include "pla_processor.hpp"

#include <fstream>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>

namespace revkit
{

bool pla_parser( std::istream& in, pla_processor& reader, bool skip_after_first_cube )
{
  std::string line;

  while ( in.good() && getline( in, line ) )
  {
    boost::trim( line );
    boost::regex_replace( line, boost::regex( "\\s+" ), " ", boost::match_default | boost::format_all );
    if ( !line.size() ) { continue; }

    if ( boost::starts_with( line, "#" ) )
    {
      std::string comment( std::find_if_not( line.begin(), line.end(), []( char c ) { return c == '#'; } ), line.end() );
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
      boost::split( inout, line, boost::is_space(), boost::token_compress_on );
      assert( inout.size() == 2 );
      reader.on_cube( inout[0], inout[1] );

      if ( skip_after_first_cube )
      {
        break;
      }
    }
  }
}

bool pla_parser( const std::string& filename, pla_processor& reader, bool skip_after_first_cube )
{
  std::ifstream is;
  is.open(filename.c_str());

  return pla_parser( is, reader, skip_after_first_cube );
}

}

// Local Variables:
// c-basic-offset: 2
// End:
