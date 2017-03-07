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
 * @file write_verilog.hpp
 *
 * @brief Writes a circuit to a Verilog file
 *
 * @author Mathias Soeken
 * @since 1.1
 */

#ifndef WRITE_VERILOG_HPP
#define WRITE_VERILOG_HPP

#include <iostream>

namespace cirkit
{

  class circuit;

  /**
   * @brief Settings for write_verilog
   *
   * @since  1.1
   */
  struct write_verilog_settings
  {
    /**
     * @brief Standard constructor
     *
     * Initializes default values
     *
     * @since  1.1
     */
    write_verilog_settings();

    /**
     * @brief Determines whether the constants are propagated
     *
     * If the constants are propagated, then no signals
     * are created for constant inputs. Instead, the values
     * are simulated by creating the Verilog file.
     *
     * The default value is \b true.
     *
     * @since  1.1
     */
    bool propagate_constants;
  };

  /**
   * @brief Writes a circuit to a Verilog file
   *
   * This function dumps the circuit as a Verilog file.
   *
   * @param circ Circuit
   * @param os Output Stream to write to
   * @param settings Settings
   *
   * @since  1.1
   */
  void write_verilog( const circuit& circ, std::ostream& os = std::cout, const write_verilog_settings& settings = write_verilog_settings() );

  /**
   * @brief Writes a circuit to a Verilog file
   *
   * This function dumps the circuit as a Verilog file.
   *
   * @param circ Circuit
   * @param filename Filename
   * @param settings Settings
   *
   * @since  2.0
   */
  void write_verilog( const circuit& circ, const std::string& filename, const write_verilog_settings& settings = write_verilog_settings() );

}

#endif /* WRITE_VERILOG_HPP */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
