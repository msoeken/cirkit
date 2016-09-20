/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2016  EPFL
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
 * @file string_template.hpp
 *
 * @brief Simple string template engine
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef STRING_TEMPLATE_HPP
#define STRING_TEMPLATE_HPP

#include <string>
#include <unordered_map>

namespace cirkit
{

class string_template
{
public:
  string_template( const std::string& str );

  std::string render( const std::unordered_map<std::string, std::string>& subst = {} ) const;

  std::string operator()( const std::unordered_map<std::string, std::string>& subst = {} ) const;

private:
  std::string str;
};

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
