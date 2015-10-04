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

/**
 * @file terminal.hpp
 *
 * @brief Some helper functions for working with terminals
 *
 * @author Mathias Soeken
 * @since  2.0
 */

#ifndef TERMINAL_HPP
#define TERMINAL_HPP

#include <iostream>
#include <string>

namespace cirkit
{

void print_banner( const std::string& caption, unsigned width = 40u );

class null_stream : public std::streambuf
{
public:
  inline int overflow( int c ) { return c; }
};

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
