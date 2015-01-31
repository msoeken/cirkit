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
