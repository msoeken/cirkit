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
 * @file print_circuit.hpp
 *
 * @brief Console output of a circuit
 *
 * @author Mathias Soeken
 * @since  1.0
 */

#ifndef PRINT_CIRCUIT_HPP
#define PRINT_CIRCUIT_HPP

#include <iostream>

#include <reversible/circuit.hpp>

namespace cirkit
{

class gate;

/**
 * @brief Settings for print_circuit function
 *
 * @note This settings cannot be applied when
 *       using operator<<(std::ostream&, const circuit&)
 *       to print a circuit.
 *
 * @since  1.0
 */
class print_circuit_settings
{
public:
  /**
   * @brief Default constructor
   *
   * Initializes default values
   *
   * @param os The stream parameter has to be determined in the constructor
   *
   * @since  1.0
   */
  print_circuit_settings( std::ostream& os = std::cout );
  virtual ~print_circuit_settings() {}

  /**
   * @brief The stream to dump the circuit to (default: \b std::ostream)
   *
   * @since  1.0
   */
  std::ostream& os;

  /**
   * @brief Determines whether the inputs and outputs should be printed (default: \b false)
   *
   * @since  1.0
   */
  bool print_inputs_and_outputs = false;

  /**
   * @brief Determines whether the gate_index should be printed (default: \b false)
   *
   * If this flag is enabled, in the first line --
   * before the circuit is printed -- the gate
   * index is printed as modulo to base 10, because
   * there is only space for one character.
   *
   * @note The first gate has the index 0.
   *
   * @since  1.0
   */
  bool print_gate_index = false;

  /**
   * @brief Character to be printed for a positive control line
   *
   * Default value is ●
   *
   * @since 1.0
   */
  std::string control_char = "●";

  /**
   * @brief Character to be printed for a negative control line
   *
   * Default value is ○
   *
   * @since 2.0
   */
  std::string negative_control_char = "○";

  /**
   * @brief Character to be printed for a STG control
   *
   * @since 2.3
   */
  std::string stg_control_char = "■";

  /**
   * @brief Character to be printed for an empty line
   *
   * Default value is \b -
   *
   * @since 1.0
   */
  std::string line_char = "―";

  /**
   * @brief Space between gates
   *
   * Default value is \b 1.
   *
   * @since 1.0
   */
  unsigned gate_spacing = 1u;

  /**
   * @brief Space between lines
   *
   * Default value is \b 0.
   *
   * @since  1.0
   */
  unsigned line_spacing = 0u;

  /**
   * @brief Returns a char for a gate
   *
   * The default implementation returns \b O for Toffoli
   * and Peres gates, \b X for Fredkin, \b v for V gates,
   * and \b + for V+ gates.
   *
   * For unknown gates an empty char ' ' is returned.
   *
   * When overriding this method, first the base method can be called
   * and further decisions can be made when the return value is an
   * empty char meaning it is an unknown type.
   *
   * @param g Gate
   *
   * @return The char
   */
  virtual std::string target_type_char( const gate& g ) const;
};

/**
 * @brief Prints a circuit as ASCII
 *
 * This method can be used to dump a circuit
 * as ASCII to the console output or into 
 * debug files.
 *
 * @param circ     Circuit
 * @param settings Settings (see print_circuit_settings)
 *
 * @since  1.0
 */
void print_circuit( const circuit& circ, const print_circuit_settings& settings = print_circuit_settings() );

/**
 * @brief Wrapper for using with the output stream operator
 *
 * This operator wraps the print_circuit method to output a circuit
 * in a stream flow using the left shift operator.
 *
 * @param os   The stream to dump the circuit
 * @param circ Circuit
 *
 * @return The stream given as parameter \p os
 *
 * @since  1.0
 */
std::ostream& operator<<( std::ostream& os, const circuit& circ );

/**
 * @brief Writes to a circuit format used by IQC
 */
void print_circuit_iqc( std::ostream& os, const circuit& circ );

/**
 * @brief I/O wrapper
 *
 * Let's you write something like
 *
 * std::cout << format_iqc( circ ) << std::endl;
 */
struct format_iqc
{
  explicit format_iqc( const circuit& circ );
  const circuit& circ;
};
std::ostream& operator<<( std::ostream& os, const format_iqc& fmt );

}

#endif /* PRINT_CIRCUIT_HPP */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
