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
 * @file circuit_from_string.hpp
 *
 * @brief Creates a reversible circuit from string
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef CIRCUIT_FROM_STRING_HPP
#define CIRCUIT_FROM_STRING_HPP

#include <string>

#include <reversible/circuit.hpp>

namespace cirkit
{

/*
 * The format is according to the REAL file format
 * but skips all the meta information.  The variable names
 * are a, b, c, d, ... and the number of lines is automatically
 * adjusted to the largest variable name.
 * Gates are separated by ; on default.
 * So far, only Toffoli gates are supported.
 */

circuit circuit_from_string( const std::string& description, const std::string& sep = ";" );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
