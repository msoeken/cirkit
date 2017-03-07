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
 * @file copy_metadata.hpp
 *
 * Copy meta-data from specification or circuit to circuit
 *
 * @author Mathias Soeken
 * @since  1.2
 */

#ifndef COPY_METADATA_HPP
#define COPY_METADATA_HPP

#include <reversible/circuit.hpp>
#include <reversible/truth_table.hpp>

namespace cirkit
{

  /**
   * @brief Settings for copy_metadata
   *
   * With this settings structure some parts can be
   * deactivated for copying. In the default case,
   * all information of the circuit is copied.
   *
   * @since  1.2
   *
   */
  struct copy_metadata_settings
  {
    /**
     * @brief Standard Constructor
     *
     * Initializes default values
     *
     * @since  1.2
     */
    copy_metadata_settings();

    /**
     * @brief Input names are copied
     *
     * The default value is \b true.
     *
     * @since  1.2
     */
    bool copy_inputs;

    /**
     * @brief Output names are copied
     *
     * The default value is \b true.
     *
     * @since  1.2
     */
    bool copy_outputs;

    /**
     * @brief Constant line information is copied
     *
     * The default value is \b true.
     *
     * @since  1.2
     */
    bool copy_constants;

    /**
     * @brief Garbage line information is copied
     *
     * The default value is \b true.
     *
     * @since  1.2
     */
    bool copy_garbage;

    /**
     * @brief Circuit name is copied
     *
     * The default value is \b true.
     *
     * @since  1.2
     */
    bool copy_name;

    /**
     * @brief Input buses are copied
     *
     * The default value is \b true.
     *
     * @since  1.2
     */
    bool copy_inputbuses;

    /**
     * @brief Output buses are copied
     *
     * The default value is \b true.
     *
     * @since  1.2
     */
    bool copy_outputbuses;

    /**
     * @brief State signals are copied
     *
     * The default value is \b true.
     *
     * @since  1.2
     */
    bool copy_statesignals;

    /**
     * @brief Modules are copied
     *
     * The default value is \b true.
     *
     * @since  1.2
     */
    bool copy_modules;
  };

  /**
   * @brief Copies meta-data from a specification to a circuit
   *
   * This method reads the inputs and outputs from a specification
   * and assigns to it to a circuit. Truth-table based synthesis
   * algorithms should use this method.
   *
   * @param spec Truth Table
   * @param circ Circuit
   *
   * @since  1.0
   */
  template<typename T>
  void copy_metadata( const truth_table<T>& spec, circuit& circ )
  {
    circ.set_inputs( spec.inputs() );
    circ.set_outputs( spec.outputs() );
    circ.set_constants( spec.constants() );
    circ.set_garbage( spec.garbage() );
  }

  /**
   * @brief Copies meta-data from a circuit to another circuit
   *
   * This method copies everything but the gates from circuit \p base
   * to circuit \p circ.
   *
   * @param base Source circuit
   * @param circ Destination circuit
   * @param settings Settings for copy_metadata (since 1.2)
   *
   * @since  1.0
   */
  void copy_metadata( const circuit& base, circuit& circ, const copy_metadata_settings& settings = copy_metadata_settings() );

}

#endif /* COPY_METADATA_HPP */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
