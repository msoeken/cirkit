/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
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

#include "string_utils.hpp"

#include <fstream>

#include <boost/algorithm/string/trim.hpp>
#include <boost/range/algorithm.hpp>

namespace cirkit
{

void split_string( std::vector<std::string>& list, const std::string& s, const std::string& delimiter )
{
  using boost::adaptors::transformed;

  if ( s.empty() ) return;

  std::string s_copy = s;
  boost::trim( s_copy );

  std::vector<std::string> str_list;
  boost::split( list, s_copy, boost::is_any_of( delimiter ), boost::algorithm::token_compress_on );
}

void foreach_string( const std::string& str, const std::string& delimiter, std::function<void(const std::string&)> func )
{
  if ( str.empty() ) return;

  std::string s_copy = str;
  boost::trim( s_copy );

  std::vector<std::string> str_list;
  boost::split( str_list, s_copy, boost::is_any_of( delimiter ), boost::algorithm::token_compress_on );
  boost::for_each( str_list, func );
}

std::pair<std::string, std::string> split_string_pair( const std::string& str, const std::string& delimiter )
{
  std::string s_copy = str;
  boost::trim( s_copy );

  std::vector<std::string> str_list;
  boost::split( str_list, s_copy, boost::is_any_of( delimiter ), boost::algorithm::token_compress_on );
  assert( str_list.size() == 2u );
  return {str_list[0u], str_list[1u]};
}


void line_parser( const std::string& filename, const std::vector<std::pair<boost::regex, std::function<void(const boost::smatch&)>>>& matchers )
{
  std::ifstream in( filename.c_str(), std::ifstream::in );
  std::string line;

  boost::smatch m;

  while ( getline( in, line ) )
  {
    for ( const auto& matcher : matchers )
    {
      if ( boost::regex_search( line, m, matcher.first ) )
      {
        matcher.second( m );
      }
    }
  }
}

void foreach_line_in_file( const std::string& filename, const std::function<void(const std::string&)>& f )
{
  std::ifstream in( filename.c_str(), std::ifstream::in );
  std::string line;

  while ( getline( in, line ) )
  {
    boost::trim( line );
    f( line );
  }
}

bool any_line_contains( const std::string& filename, const boost::regex& r )
{
  bool ret = false;

  line_parser( filename, { { r, [&]( const boost::smatch& m ) { ret = true; } } } );

  return ret;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
