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
 * @file expression_parser.hpp
 *
 * @brief Expression parser
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef EXPRESSION_PARSER_HPP
#define EXPRESSION_PARSER_HPP

#include <deque>
#include <iostream>
#include <memory>
#include <string>

namespace cirkit
{

struct expression_t
{
  using ptr = std::shared_ptr<expression_t>;
  enum type_t { _const, _var, _inv, _and, _or, _maj, _xor };

  type_t          type;
  unsigned        value;
  std::deque<ptr> children;
};

expression_t::ptr parse_expression( const std::string& expression );
std::ostream& operator<<( std::ostream& os, const expression_t::ptr& expr );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
