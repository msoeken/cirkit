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
 * @file write_realization.hpp
 *
 * @brief Generator for RevLib realization (*.real) format
 *
 * @author Mathias Soeken
 * @since  1.0
 */

#ifndef WRITE_REALIZATION_HPP
#define WRITE_REALIZATION_HPP

#include <iosfwd>
#include <string>

#include <reversible/circuit.hpp>

namespace cirkit
{

  /**
   * @brief Settings for write_realization function
   *
   * @since  1.0
   */
  struct write_realization_settings
  {
    /**
     * @brief Default constructor
     *
     * Initializes default values
     *
     * @since  1.0
     */
    write_realization_settings();

    /**
     * @brief A version string
     *
     * Default value is 2.0 and printed after \b .version command
     *
     * @since  1.0
     */
    std::string version = "2.0";

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
     * circuit circ;
     *
     * write_realization_settings settings;
     * settings.header = "Author: Test User\n(c) University";
     * write_realization( circ, "circuit.real", settings );
     * @endcode
     *
     * @since  1.0
     */
    std::string header;

    virtual std::string type_label( const gate& g ) const;
  };

  /**
   * @brief Writes a circuit as RevLib realization to an output stream
   *
   * This method takes a circuit and writes it to an output stream.
   * Custom settings can be controlled via the \p settings parameter,
   * which has default values if not set explicitely.
   *
   * It may be more convenient to use the write_realization(const circuit&, const std::string&, const write_realization_settings&, std::string*)
   * function which takes a filename as parameter instead of an output stream.
   *
   * This function can be useful when dumping to e.g. STDOUT.
   *
   * @section Example
   * The following example writes a circuit to STDOUT.
   *
   * @code
   * circuit circ;
   * write_realization( circ, std::cout );
   * @endcode
   *
   * @param circ     Circuit to write
   * @param os       Output stream
   * @param settings Settings (see write_realization_settings)
   *
   * @since  1.0
   */
  void write_realization( const circuit& circ, std::ostream& os, const write_realization_settings& settings = write_realization_settings() );

  /**
   * @brief Writes a circuit as RevLib realization to a file
   *
   * This is a wrapper function for write_realization(const circuit&, std::ostream&, const write_realization_settings&)
   * and takes a \p filename as paramter. The forth parameter is a pointer
   * to a string which can contain an error message in case the function
   * call fails. This can only be the case when the file cannot be
   * opened for writing.
   *
   * @section Example
   * The following example reads a realization from an existing file,
   * changes its version and header and writes
   * it back to the the same file.
   * @code
   * circuit circ;
   * std::string filename = "circuit.real";
   * std::string error;
   *
   * if ( !read_realization( circ, filename, &error ) )
   * {
   *   std::cerr << error << std::endl;
   *   exit( 1 );
   * }
   *
   * write_realization_settings settings;
   * settings.version = "2.0";
   * settings.header = "Reversible Benchmarks\n(c) University";
   *
   * if ( !write_realization( circ, filename, settings, &error ) )
   * {
   *   std::cerr << error << std::endl;
   * }
   * @endcode
   *
   * @param circ     Circuit to write
   * @param filename Filename of the file to be created. The fill will be
   *                 overwritten in case it is already existing
   * @param settings Settings (see write_realization_settings)
   * @param error    If not-null, an error message is written
   *                 to this parameter in case the function fails
   *
   * @return true on success, false otherwise
   *
   * @since  1.0
   */
  bool write_realization( const circuit& circ, const std::string& filename, const write_realization_settings& settings = write_realization_settings(), std::string* error = 0 );

}

#endif /* WRITE_REALIZATION_HPP */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
