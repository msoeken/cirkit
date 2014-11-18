/* RevKit (www.revkit.org)
 * Copyright (C) 2009-2014  University of Bremen
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

void line_parser( const std::string& filename, const std::vector<std::pair<std::regex, std::function<void(const std::smatch&)>>>& matchers );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
