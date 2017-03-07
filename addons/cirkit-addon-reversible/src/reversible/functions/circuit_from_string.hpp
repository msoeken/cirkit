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
 * Gates are separated by , on default.
 * So far, only Toffoli and Fredkin gates are supported.
 */

circuit circuit_from_string( const std::string& description, const std::string& sep = "," );

std::string circuit_to_string( const circuit& circ, const std::string& sep = "," );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
