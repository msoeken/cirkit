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

#include "string_utils.hpp"

#include <fstream>
#include <iostream>

#include <boost/assign/std/vector.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/format.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
#include <boost/tokenizer.hpp>

using namespace boost::assign;

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

void line_parser( const std::string& filename, const std::vector<std::pair<std::regex, std::function<void(const std::smatch&)>>>& matchers, bool warn_if_unmatched )
{
  std::ifstream in( filename.c_str(), std::ifstream::in );
  std::string line;

  std::smatch m;

  while ( getline( in, line ) )
  {
    boost::trim( line );
    bool matched = false;
    for ( const auto& matcher : matchers )
    {
      if ( std::regex_search( line, m, matcher.first ) )
      {
        matcher.second( m );
        matched = true;
        break;
      }
    }
    if ( !matched && warn_if_unmatched )
    {
      std::cout << "[w] could not match " << line << std::endl;
    }
  }
}

void foreach_line_in_file( const std::string& filename, const std::function<bool(const std::string&)>& f )
{
  std::ifstream in( filename.c_str(), std::ifstream::in );
  std::string line;

  while ( getline( in, line ) )
  {
    boost::trim( line );
    if ( !f( line ) ) { break; }
  }
}

void foreach_line_in_file_escape( const std::string& filename, const std::function<bool(const std::string&)>& f )
{
  std::ifstream in( filename.c_str(), std::ifstream::in );
  std::string line, line2;

  while ( getline( in, line ) )
  {
    boost::trim( line );

    while ( line.back() == '\\' )
    {
      line.pop_back();
      boost::trim( line );
      assert( getline( in, line2 ) );
      line += line2;
    }

    if ( !f( line ) ) { break; }
  }
}

bool any_line_contains( const std::string& filename, const std::regex& r )
{
  bool ret = false;

  line_parser( filename, { { r, [&]( const std::smatch& m ) { ret = true; } } } );

  return ret;
}

std::string make_properties_string( const string_properties_map_t& properties, const std::string& sep )
{
  using boost::adaptors::transformed;

  return boost::join( properties | transformed( []( const std::pair<std::string, std::string>& p ) { return boost::str( boost::format( "%s=%s" ) % p.first % p.second ); } ), sep );
}

std::vector<std::string> split_with_quotes( const std::string& s )
{
  std::vector<std::string> result;
  boost::tokenizer<boost::escaped_list_separator<char>> tok( s, boost::escaped_list_separator<char>( '\\', ' ', '\"' ) );

  for ( const auto& s : tok )
  {
    if ( !s.empty() )
    {
      result += s;
    }
  }
  return result;
}


}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
