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
 * @file add_gates.hpp
 *
 * @brief Adding typical gates to a circuit
 *
 * @author Mathias Soeken
 * @author Stefan Frehse
 * @since  1.0
 */

#ifndef ADD_GATES_HPP
#define ADD_GATES_HPP

#include <reversible/circuit.hpp>

#include <boost/dynamic_bitset.hpp>

namespace cirkit
{

/**
 * @brief Helper class for adding lines in an easier way
 *
 * This class should not be used stand alone but just with the add_\em gate methods
 * designed for this purpose. See also \ref sub_add_gates.
 *
 * @since  1.0
 *
 * @sa \ref sub_add_gates
 */
class target_line_adder
{
public:
  /**
   * @brief Default constructor
   *
   * @param gate Gate, to which target lines should be added
   *
   * @sa \ref sub_add_gates
   *
   * @since  1.0
   */
  explicit target_line_adder( gate* gate );

  /**
   * @brief Add one target line
   *
   * @param  l1 First target line
   *
   * @return A smart pointer to the gate
   *
   * @sa \ref sub_add_gates
   *
   * @since  1.0
   */
  gate& operator()( unsigned l1 );

  /**
   * @brief Add two target lines
   *
   * @param  l1 First target line
   * @param  l2 Second target line
   *
   * @return A smart pointer to the gate
   *
   * @sa \ref sub_add_gates
   *
   * @since  1.0
   */
  gate& operator()( unsigned l1, unsigned l2 );

private:
  gate* g;
};

/**
 * @brief Helper class for adding lines in an easier way
 *
 * This class should not be used stand alone but just with the add_\em gate methods
 * designed for this purpose. See also \ref sub_add_gates.
 *
 * @since  1.0
 *
 * @sa \ref sub_add_gates
 */
class control_line_adder
{
public:
  /**
   * @brief Default constructor
   *
   * @param g Gate, to which control lines should be added
   *
   * @sa \ref sub_add_gates
   *
   * @since  1.0
   */
  explicit control_line_adder( gate& g );

  /**
   * @brief Add up to nine control lines
   *
   * @param  l1 First control line
   * @param  l2 Second control line
   * @param  l3 Second control line
   * @param  l4 Fourth control line
   * @param  l5 Fifth control line
   * @param  l6 Sixth control line
   * @param  l7 Seventh control line
   * @param  l8 Eighth control line
   * @param  l9 Ninth control line
   *
   * @return A target_line_adder
   *
   * @sa \ref sub_add_gates
   *
   * @since  2.0
   */
  target_line_adder operator()( boost::optional<variable> l1 = boost::optional<variable>(),
                                boost::optional<variable> l2 = boost::optional<variable>(),
                                boost::optional<variable> l3 = boost::optional<variable>(),
                                boost::optional<variable> l4 = boost::optional<variable>(),
                                boost::optional<variable> l5 = boost::optional<variable>(),
                                boost::optional<variable> l6 = boost::optional<variable>(),
                                boost::optional<variable> l7 = boost::optional<variable>(),
                                boost::optional<variable> l8 = boost::optional<variable>(),
                                boost::optional<variable> l9 = boost::optional<variable>() );

  /**
   * @brief Add up to nine positive control lines
   *
   * @param  l1 First control line
   * @param  l2 Second control line
   * @param  l3 Second control line
   * @param  l4 Fourth control line
   * @param  l5 Fifth control line
   * @param  l6 Sixth control line
   * @param  l7 Seventh control line
   * @param  l8 Eighth control line
   * @param  l9 Ninth control line
   *
   * @return A target_line_adder
   *
   * @sa \ref sub_add_gates
   *
   * @since  2.0
   */
  target_line_adder operator()( boost::optional<unsigned> l1 = boost::optional<unsigned>(),
                                boost::optional<unsigned> l2 = boost::optional<unsigned>(),
                                boost::optional<unsigned> l3 = boost::optional<unsigned>(),
                                boost::optional<unsigned> l4 = boost::optional<unsigned>(),
                                boost::optional<unsigned> l5 = boost::optional<unsigned>(),
                                boost::optional<unsigned> l6 = boost::optional<unsigned>(),
                                boost::optional<unsigned> l7 = boost::optional<unsigned>(),
                                boost::optional<unsigned> l8 = boost::optional<unsigned>(),
                                boost::optional<unsigned> l9 = boost::optional<unsigned>() );

private:
  gate* g;
};

/**
 * @brief Helper function for appending a \b Toffoli gate
 *
 * @param circ     Circuit
 * @param controls Control Lines
 * @param target   Target Line
 *
 * @return Gate reference
 *
 * @since  1.0
 */
gate& append_toffoli( circuit& circ, const gate::control_container& controls, unsigned target );
gate& append_toffoli( circuit& circ, const std::vector<unsigned>& controls, unsigned target );

/**
 * @brief Helper function for appending a \b Fredkin gate
 *
 * @param circ     Circuit
 * @param controls Control Lines
 * @param target1  Target Line 1
 * @param target2  Target Line 2
 *
 * @return Gate reference
 *
 * @since  1.0
 */
gate& append_fredkin( circuit& circ, const gate::control_container& controls, unsigned target1, unsigned target2 );

/**
 * @brief Helper function for appending a \b Peres gate
 *
 * @param circ    Circuit
 * @param control Control Line
 * @param target1 Target Line 1
 * @param target2 Target Line 2
 *
 * @return Gate reference
 *
 * @since  1.0
 */
gate& append_peres( circuit& circ, variable control, unsigned target1, unsigned target2 );

/**
 * @brief Helper function for appending a \b CNOT gate
 *
 * @param circ    Circuit
 * @param control Control Line
 * @param target  Target Line
 *
 * @return Gate reference
 *
 * @since  1.0
 */
gate& append_cnot( circuit& circ, variable control, unsigned target );
gate& append_cnot( circuit& circ, unsigned control, unsigned target );

/**
 * @brief Helper function for appending a \b NOT gate
 *
 * @param circ    Circuit
 * @param target  Target Line
 *
 * @return Gate reference
 *
 * @since  1.0
 */
gate& append_not( circuit& circ, unsigned target );

/**
 * @brief Helper function for appending a module gate
 *
 * @param circ        Circuit
 * @param module_name Name of the module (already added to the circuit)
 * @param controls    Control Lines
 * @param targets     Target Lines (has to be a ordered vector for the mapping to the module lines)
 *
 * @return Gate reference
 *
 * @since  1.1
 */
gate& append_module( circuit& circ, const std::string& module_name, const gate::control_container& controls, const gate::target_container& targets );

/**
 * @brief Helper function for appending a generic gate using the control_line_adder
 *
 * @param circ Circuit
 * @param tag  Gate type tag
 *
 * @return A control_line_adder
 *
 * @sa \ref sub_add_gates
 * @sa \ref sub_target_tags
 *
 * @since  1.0
 */
control_line_adder append_gate( circuit& circ, const boost::any& tag );

/**
 * @brief Helper function for appending a \b Toffoli gate using the control_line_adder
 *
 * @param circ Circuit
 *
 * @return A control_line_adder
 *
 * @sa \ref sub_add_gates
 *
 * @since  1.0
 */
control_line_adder append_toffoli( circuit& circ );

/**
 * @brief Helper function for appending a \b Fredkin gate using the control_line_adder
 *
 * @param circ Circuit
 *
 * @return A control_line_adder
 *
 * @sa \ref sub_add_gates
 *
 * @since  1.0
 */
control_line_adder append_fredkin( circuit& circ );

gate& append_stg( circuit& circ, const boost::dynamic_bitset<>& function, const gate::control_container& controls, unsigned target );


/**
 * @brief Helper function for prepending a \b Toffoli gate
 *
 * @param circ     Circuit
 * @param controls Control Lines
 * @param target   Target Line
 *
 * @return Gate reference
 *
 * @since  1.0
 */
gate& prepend_toffoli( circuit& circ, const gate::control_container& controls, unsigned target );
gate& prepend_toffoli( circuit& circ, const std::vector<unsigned>& controls, unsigned target );

/**
 * @brief Helper function for prepending a \b Fredkin gate
 *
 * @param circ     Circuit
 * @param controls Control Lines
 * @param target1  Target Line 1
 * @param target2  Target Line 2
 *
 * @return Gate reference
 *
 * @since  1.0
 */
gate& prepend_fredkin( circuit& circ, const gate::control_container& controls, unsigned target1, unsigned target2 );

/**
 * @brief Helper function for prepending a \b Peres gate
 *
 * @param circ    Circuit
 * @param control Control Line
 * @param target1 Target Line 1
 * @param target2 Target Line 2
 *
 * @return Gate reference
 *
 * @since  1.0
 */
gate& prepend_peres( circuit& circ, variable control, unsigned target1, unsigned target2 );

/**
 * @brief Helper function for prepending a \b CNOT gate
 *
 * @param circ    Circuit
 * @param control Control Line
 * @param target  Target Line
 *
 * @return Gate reference
 *
 * @since  1.0
 */
gate& prepend_cnot( circuit& circ, variable control, unsigned target );
gate& prepend_cnot( circuit& circ, unsigned control, unsigned target );

/**
 * @brief Helper function for prepending a \b NOT gate
 *
 * @param circ    Circuit
 * @param target  Target Line
 *
 * @return Gate reference
 *
 * @since  1.0
 */
gate& prepend_not( circuit& circ, unsigned target );

/**
 * @brief Helper function for prepending a module gate
 *
 * @param circ        Circuit
 * @param module_name Name of the module (already added to the circuit)
 * @param controls    Control Lines
 * @param targets     Target Lines (has to be a ordered vector for the mapping to the module lines)
 *
 * @return Gate reference
 *
 * @since  1.1
 */
gate& prepend_module( circuit& circ, const std::string& module_name, const gate::control_container& controls, const gate::target_container& targets );

/**
 * @brief Helper function for prepending a generic gate using the control_line_adder
 *
 * @param circ Circuit
 * @param tag  Gate type tag
 *
 * @return A control_line_adder
 *
 * @sa \ref sub_add_gates
 * @sa \ref sub_target_tags
 *
 * @since  1.0
 */
control_line_adder prepend_gate( circuit& circ, const boost::any& tag );

/**
 * @brief Helper function for prepending a \b Toffoli gate using the control_line_adder
 *
 * @param circ Circuit
 *
 * @return A control_line_adder
 *
 * @sa \ref sub_add_gates
 *
 * @since  1.0
 */
control_line_adder prepend_toffoli( circuit& circ );

/**
 * @brief Helper function for prepending a \b Fredkin gate using the control_line_adder
 *
 * @param circ Circuit
 *
 * @return A control_line_adder
 *
 * @sa \ref sub_add_gates
 *
 * @since  1.0
 */
control_line_adder prepend_fredkin( circuit& circ );

gate& prepend_stg( circuit& circ, const boost::dynamic_bitset<>& function, const gate::control_container& controls, unsigned target );



/**
 * @brief Helper function for inserting a \b Toffoli gate
 *
 * @param circ     Circuit
 * @param n        Index to insert the gate
 * @param controls Control Lines
 * @param target   Target Line
 *
 * @return Gate reference
 *
 * @since  1.0
 */
gate& insert_toffoli( circuit& circ, unsigned n, const gate::control_container& controls, unsigned target );
gate& insert_toffoli( circuit& circ, unsigned n, const std::vector<unsigned>& controls, unsigned target );

/**
 * @brief Helper function for inserting a \b Fredkin gate
 *
 * @param circ     Circuit
 * @param n        Index to insert the gate
 * @param controls Control Lines
 * @param target1  Target Line 1
 * @param target2  Target Line 2
 *
 * @return Gate reference
 *
 * @since  1.0
 */
gate& insert_fredkin( circuit& circ, unsigned n, const gate::control_container& controls, unsigned target1, unsigned target2 );

/**
 * @brief Helper function for inserting a \b Peres gate
 *
 * @param circ    Circuit
 * @param n       Index to insert the gate
 * @param control Control Line
 * @param target1 Target Line 1
 * @param target2 Target Line 2
 *
 * @return Gate reference
 *
 * @since  1.0
 */
gate& insert_peres( circuit& circ, unsigned n, variable control, unsigned target1, unsigned target2 );

/**
 * @brief Helper function for inserting a \b CNOT gate
 *
 * @param circ    Circuit
 * @param n       Index to insert the gate
 * @param control Control Line
 * @param target  Target Line
 *
 * @return Gate reference
 *
 * @since  1.0
 */
gate& insert_cnot( circuit& circ, unsigned n, variable control, unsigned target );
gate& insert_cnot( circuit& circ, unsigned n, unsigned control, unsigned target );

/**
 * @brief Helper function for inserting a \b NOT gate
 *
 * @param circ    Circuit
 * @param n       Index to insert the gate
 * @param target  Target Line
 *
 * @return Gate reference
 *
 * @since  1.0
 */
gate& insert_not( circuit& circ, unsigned n, unsigned target );

/**
 * @brief Helper function for inserting a module gate
 *
 * @param circ        Circuit
 * @param n           Index to insert the gate
 * @param module_name Name of the module (already added to the circuit)
 * @param controls    Control Lines
 * @param targets     Target Lines (has to be a ordered vector for the mapping to the module lines)
 *
 * @return Gate reference
 *
 * @since  1.1
 */
gate& insert_module( circuit& circ, unsigned n, const std::string& module_name, const gate::control_container& controls, const gate::target_container& targets );

/**
 * @brief Helper function for inserting a generic gate using the control_line_adder
 *
 * @param circ Circuit
 * @param n    Index to insert the gate
 * @param tag  Gate type tag
 *
 * @return A control_line_adder
 *
 * @sa \ref sub_add_gates
 * @sa \ref sub_target_tags
 *
 * @since  1.0
 */
control_line_adder insert_gate( circuit& circ, unsigned n, const boost::any& tag );

/**
 * @brief Helper function for inserting a \b Toffoli gate using the control_line_adder
 *
 * @param circ Circuit
 * @param n    Index to insert the gate
 *
 * @return A control_line_adder
 *
 * @sa \ref sub_add_gates
 *
 * @since  1.0
 */
control_line_adder insert_toffoli( circuit& circ, unsigned n );

/**
 * @brief Helper function for inserting a \b Fredkin gate using the control_line_adder
 *
 * @param circ Circuit
 * @param n    Index to insert the gate
 *
 * @return A control_line_adder
 *
 * @sa \ref sub_add_gates
 *
 * @since  1.0
 */
control_line_adder insert_fredkin( circuit& circ, unsigned n );

gate& insert_stg( circuit& circ, unsigned n, const boost::dynamic_bitset<>& function, const gate::control_container& controls, unsigned target );


}

#endif /* ADD_GATES_HPP */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
