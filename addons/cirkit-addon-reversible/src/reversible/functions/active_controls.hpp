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
 * @file active_controls.hpp
 *
 * @brief Slot for adding control lines automatically
 *
 * @author Mathias Soeken
 * @since  1.1
 */

#ifndef ACTIVE_CONTROLS_HPP
#define ACTIVE_CONTROLS_HPP

#include <reversible/gate.hpp>

namespace cirkit
{

  /**
   * @brief Slot for adding control lines automatically
   *
   * This is a slot which can be connected to the circuit::gate_added
   * signal in order to add special control lines automatically.
   * These are called active controls. For example a block of gates should
   * be added which is only active when a particular control line is
   * activated.
   *
   * This control line can be activatd with this class and then the
   * class has to be connected to the signal of the circuit.
   *
   * @section example_active_controls Example
   * The code below seems to add a CNOT and a NOT gate.
   * However, since the controller is connected and the
   * line 0 is added to the controller each time also this
   * control line is activated and, thus, a Toffoli gate
   * and a CNOT gate are added.
   * @code
   * #include <reversible/circuit.hpp>
   * #include <reversible/functions/active_controls.hpp>
   * #include <reversible/functions/add_gates.hpp>
   *
   * revkit::circuit circ( 3 );
   *
   * revkit::active_controls controller;
   * controller.add( 0u );
   *
   * circ.gate_added.connect( controller );
   *
   * revkit::append_cnot( circ, 1u, 2u );
   * revkit::append_not( circ, 1u );
   * @endcode
   */
  class active_controls
  {
  public:
    active_controls();
    ~active_controls();

    /**
     * @brief Add an active control line
     *
     * An active control line is implicitely added by every operation
     * which adds a new gate to the circuit. This is especially useful
     * in hierarchical synthesis approaches.
     *
     * This method does not add a line, it only sets an existing
     * line to be active.
     *
     * @param control Index of the line which should be active
     *
     * @since  1.1
     */
    void add( variable control );

    /**
     * @brief Removes an active control line
     *
     * An active control line is implicitely added by every operation
     * which adds a new gate to the circuit. This is especially useful
     * in hierarchical synthesis approaches.
     *
     * This method does not remove a line, it only unsets an existing
     * line to be active.
     *
     * @param control Index of the line which should be deactivated
     *
     * @since  1.1
     */
    void remove( variable control );

    /**
     * @brief Returns a list with all active lines
     *
     * An active control line is implicitely added by every operation
     * which adds a new gate to the circuit. This is especially useful
     * in hierarchical synthesis approaches.
     *
     * @return List with all active lines
     *
     * @since  1.1
     */
    const gate::control_container& controls() const;

    /**
     * @brief Operator implementation
     *
     * This operator adds the active controls to the gate
     * after it is added to a circuit.
     *
     * @param g Gate to be changed
     *
     * @since  1.1
     */
    void operator()( gate& g ) const;

  private:
    class priv;
    priv* const d;
  };

}

#endif /* ACTIVE_CONTROLS_HPP */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
