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

/**
 * @file string_utils.hpp
 *
 * @brief Some helper functions for strings
 *
 * @author Mathias Soeken
 * @since  2.1
 */

#ifndef STRING_UTILS_HPP
#define STRING_UTILS_HPP

#include <functional>
#include <regex>
#include <string>
#include <vector>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
#include <boost/range/adaptors.hpp>

namespace cirkit
{

template<typename T>
void parse_string_list( std::vector<T>& list, const std::string& s, const std::string& delimiter = " " )
{
  using boost::adaptors::transformed;

  if ( s.empty() ) return;

  std::string s_copy = s;
  boost::trim( s_copy );

  std::vector<std::string> str_list;
  boost::split( str_list, s_copy, boost::is_any_of( delimiter ), boost::algorithm::token_compress_on );
  boost::push_back( list, str_list | transformed( []( const std::string& s ) { return boost::lexical_cast<T>( s ); } ) );
}

void split_string( std::vector<std::string>& list, const std::string& s, const std::string& delimiter );

void foreach_string( const std::string& str, const std::string& delimiter, std::function<void(const std::string&)> func );

std::pair<std::string, std::string> split_string_pair( const std::string& str, const std::string& delimiter );

void line_parser( const std::string& filename, const std::vector<std::pair<std::regex, std::function<void(const std::smatch&)>>>& matchers, bool warn_if_unmatched = false );

void foreach_line_in_file( const std::string& filename, const std::function<bool(const std::string&)>& f );

void foreach_line_in_file_escape( const std::string& filename, const std::function<bool(const std::string&)>& f );

bool any_line_contains( const std::string& filename, const std::regex& r );

using string_properties_map_t = std::map<std::string, std::string>;
std::string make_properties_string( const string_properties_map_t& properties, const std::string& sep = "," );

std::vector<std::string> split_with_quotes( const std::string& s );

inline const std::string& empty_default( const std::string& s, const std::string& d )
{
  return s.empty() ? d : s;
}

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
