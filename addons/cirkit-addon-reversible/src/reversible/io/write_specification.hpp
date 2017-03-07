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
 * @file write_specification.hpp
 *
 * @brief Writes a truth table to a RevLib specification file
 *
 * @author Mathias Soeken
 * @since  1.0
 */

#ifndef WRITE_SPECIFICATION_HPP
#define WRITE_SPECIFICATION_HPP

#include <string>

#include <reversible/truth_table.hpp>

namespace cirkit
{

  /**
   * @brief Settings for write_specification
   *
   * @since  1.0
   */
  struct write_specification_settings
  {
    /**
     * @brief Default constructor
     *
     * Initializes default values
     *
     * @since  1.0
     */
    write_specification_settings();

    /**
     * @brief A version string
     *
     * Default value is 2.0 and printed after \b .version command
     *
     * @since  1.0
     */
    std::string version;

    /**
     * @brief A header for the file
     *
     * This header will be printed as a comment in the first
     * lines of the file. The string can be multi-line seperated
     * by \\n escape sequences. The comment character # can be
     * omitted and will be inserted automatically.
     *
     * @section Example
     * The following code creates a header in beginning of the
     * file with author information.
     *
     * @code
     * binary_truth_table spec;
     *
     * write_specification_settings settings;
     * settings.header = "Author: Test User\n(c) University";
     * write_specification( spec, "circuit.real", settings );
     * @endcode
     *
     * @since  1.0
     */
    std::string header;

    /**
     * @brief Order of literals in the output cubes
     *
     * The order of the literals in the output cube can
     * be changed with this vector. If not empty, it has to
     * have the same size as the number of outputs in the specification.
     *
     * It contains of distinct numbers from \em 0 to <i>n-1</i>, where \em n
     * is the size of the input cubes. The numbers assign the literal
     * in the output cube to that index. Empty indices (if the output cubes
     * are smaller than the input cubes) are filled with don't care
     * values.
     *
     * Default value is an empty vector.
     *
     * @since  1.0
     */
    std::vector<unsigned> output_order;
  };

  /**
   * @brief Writes a truth table to a RevLib specification file
   *
   * @param spec Specification
   * @param filename File-name to write the specification to
   * @param settings Settings
   * @return true on success, false otherwise
   */
  bool write_specification( const binary_truth_table& spec, const std::string& filename, const write_specification_settings& settings = write_specification_settings() );

}

#endif /* WRITE_SPECIFICATION_HPP */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
