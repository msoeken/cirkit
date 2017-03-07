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
 * @file revlib_parser.hpp
 *
 * @brief RevLib file format parser
 *
 * @author Mathias Soeken
 * @since  1.0
 */

#ifndef REVLIB_PARSER_HPP
#define REVLIB_PARSER_HPP

#include <functional>
#include <iosfwd>
#include <string>

#include <boost/any.hpp>
#include <boost/optional.hpp>

namespace cirkit
{

class revlib_processor;

boost::optional<boost::any> revlib_parser_string_to_target_tag( const std::string& str );

struct revlib_parser_settings
{
  /**
   * @brief A base directory to look for included files.
   *        This is usually the directory of the loaded file-name.
   */
  std::string base_directory = ".";

  /**
   * @brief Decides whether gates should be parsed. Useful when only meta-data is required.
   */
  bool read_gates = true;

  std::function<boost::optional<boost::any>(const std::string&)> string_to_target_tag = revlib_parser_string_to_target_tag;
};

/**
 * @brief A parser for the RevLib file format
 *
 * This function can read a circuit realization and truth table
 * specifications in a format as proposed on RevLib.
 * The function itself just realizes the
 * parser and uses a revlib_processor instance which
 * provides methods for implementation details.
 *
 * For example, when reading the realization into a circuit
 * structure, the read_realization_circuit_processor class can
 * be used as instance.
 *
 * @param in     Input stream containing the file
 * @param reader An instance of the revlib_processor
 * @param error A pointer to a string. In case the parsing fails,
 *              and \p error is not null, a error message is stored
 *
 * @return true on success, false otherwise
 *
 * @since  1.1
 */
bool revlib_parser( std::istream& in, revlib_processor& reader, const revlib_parser_settings& settings = revlib_parser_settings(), std::string* error = 0 );

}

#endif /* REVLIB_PARSER_HPP */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
