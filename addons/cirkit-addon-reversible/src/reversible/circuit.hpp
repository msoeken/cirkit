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
 * @file circuit.hpp
 *
 * @brief Circuit class
 *
 * @author Mathias Soeken
 * @since  1.0
 */

#ifndef CIRCUIT_HPP
#define CIRCUIT_HPP

#include <map>
#include <memory>

#include <boost/format.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/iterator/indirect_iterator.hpp>
#include <boost/optional.hpp>

#include <reversible/gate.hpp>
#include <reversible/meta/bus_collection.hpp>

namespace cirkit
{
  /**
   * @brief Type for determine whether line is constant or not
   *
   * The following table summarizes the use of constant values
   * in circuit representations.
   *
   * <table border="0">
   *   <tr>
   *     <td class="indexkey">Description</td>
   *     <td class="indexkey">Char representation</td>
   *     <td class="indexkey">Typed value</td>
   *   </tr>
   *   <tr>
   *     <td class="indexvalue">No constant input line</td>
   *     <td align="center" class="indexvalue">'-'</td>
   *     <td class="indexvalue">@code constant() @endcode</td>
   *   </tr>
   *   <tr>
   *     <td class="indexvalue">Constant input line with value 0</td>
   *     <td align="center" class="indexvalue">'0'</td>
   *     <td class="indexvalue">@code constant( 0 ) @endcode</td>
   *   </tr>
   *   <tr>
   *     <td class="indexvalue">Constant input line with value 1</td>
   *     <td align="center" class="indexvalue">'1'</td>
   *     <td class="indexvalue">@code constant( 1 ) @endcode</td>
   *   </tr>
   * </table>
   *
   * @section Example
   * This example demonstrates how to access the constant values.
   * @code
   * constant c = // some constant
   *
   * if ( c ) // checks whether c is set or not
   * {
   *   if ( *c ) // c is checked, checks the value of c
   *   {
   *     std::cout << "constant value 1" << std::endl;
   *   }
   *   else
   *   {
   *     std::cout << "constant value 0" << std::endl;
   *   }
   * }
   * else
   * {
   *   std::cout << "no constant value" << std::endl;
   * }
   * @endcode
   *
   * @since  1.0
   */
  using constant = boost::optional<bool>;

  /**
   * @brief Main circuit class
   *
   * This class represents a circuit and can be used generically for standard circuits and sub circuits.
   *
   * @since  1.0
   */
  class circuit {
  public:
    /**
     * @brief Default Constructor
     *
     * Creates an empty circuit with zero lines.
     *
     * @since  1.0
     */
    circuit() : _lines( 0 ) {}

    /**
     * @brief Default Constructor
     *
     * Creates an empty circuit with \p lines lines.
     *
     * @param lines Number of lines
     *
     * @since  1.0
     */
    circuit( unsigned lines )
    {
      set_lines( lines );
    }

    /**
     * @brief Mutable iterator for accessing the gates in a circuit
     */
    using iterator = boost::indirect_iterator<std::vector<std::shared_ptr<gate>>::iterator>;

    /**
     * @brief Constant iterator for accessing the gates in a circuit
     */
    using const_iterator = boost::indirect_iterator<std::vector<std::shared_ptr<gate>>::const_iterator>;

    /**
     * @brief Mutable reverse iterator for accessing the gates in a circuit
     */
    using reverse_iterator = boost::indirect_iterator<std::vector<std::shared_ptr<gate>>::reverse_iterator>;
    /**
     * @brief Constant reverse iterator for accessing the gates in a circuit
     */
    using const_reverse_iterator = boost::indirect_iterator<std::vector<std::shared_ptr<gate>>::const_reverse_iterator>;

    /**
     * @brief Returns the number of gates
     *
     * This method returns the number of gates in the circuit.
     *
     * @return Number of gates
     *
     * @since  1.0
     */
    unsigned num_gates() const;

    /**
     * @brief Sets the number of line
     *
     * This method sets the number of lines of the circuit.
     *
     * Changing this number will not affect the data in the gates.
     * For example: If there is a gate with a control on line 3,
     * and the number of lines is reduced to 2 in the circuit, then
     * the control is still on line 3 but not visible in this circuit.
     *
     * So, changing the lines after already adding gates can lead
     * to invalid gates.
     *
     * @param lines Number of lines
     *
     * @since  1.0
     */
    void set_lines( unsigned lines );

    /**
     * @brief Returns the number of lines
     *
     * This method returns the number of lines.
     *
     * @return Number of lines
     *
     * @since  1.0
     */
    unsigned lines() const;

    /**
     * @brief Constant begin iterator pointing to gates
     *
     * @return Constant begin iterator
     *
     * @since  1.0
     */
    const_iterator begin() const;

    /**
     * @brief Constant end iterator pointing to gates
     *
     * @return Constant end iterator
     *
     * @since  1.0
     */
    const_iterator end() const;

    /**
     * @brief Mutable begin iterator pointing to gates
     *
     * @return Mutable begin iterator
     *
     * @since  1.0
     */
    iterator begin();

    /**
     * @brief Mutable end iterator pointing to gates
     *
     * @return Mutable end iterator
     *
     * @since  1.0
     */
    iterator end();

    /**
     * @brief Constant begin reverse iterator pointing to gates
     *
     * @return Constant begin reverse iterator
     *
     * @since  1.0
     */
    const_reverse_iterator rbegin() const;

    /**
     * @brief Constant end reverse iterator pointing to gates
     *
     * @return Constant end reverse iterator
     *
     * @since  1.0
     */
    const_reverse_iterator rend() const;

    /**
     * @brief Mutable begin reverse iterator pointing to gates
     *
     * @return Mutable begin reverse iterator
     *
     * @since  1.0
     */
    reverse_iterator rbegin();

    /**
     * @brief Mutable end reverse iterator pointing to gates
     *
     * @return Mutable end reverse iterator
     *
     * @since  1.0
     */
    reverse_iterator rend();

    /**
     * @brief Random access operator for access to gates by index
     *
     * @param index Index of the gate, starting from 0
     * @return constant access to the \p index gate in the circuit
     *
     * @since  1.1
     */
    const gate& operator[]( unsigned index ) const;

    /**
     * @brief Random access operator for access to gates by index
     *
     * @param index Index of the gate, starting from 0
     * @return mutable access to the \p index gate in the circuit
     *
     * @since  1.1
     */
    gate& operator[]( unsigned index );

    /**
     * @brief Inserts a gate at the end of the circuit
     *
     * This method inserts a gate at the end of the circuit.
     *
     * @return Reference to the newly created empty gate
     *
     * @since  1.0
     */
    gate& append_gate();

    /**
     * @brief Inserts a gate at the beginning of the circuit
     *
     * This method inserts a gate at the beginning of the circuit.
     *
     * @return Reference to the newly created empty gate
     *
     * @since  1.0
     */
    gate& prepend_gate();

    /**
     * @brief Inserts a gate into the circuit
     *
     * This method inserts a gate at an arbitrary position in the circuit
     *
     * @param pos  Position where to insert the gate
     *
     * @return Reference to the newly created empty gate
     *
     * @since  1.0
     */
    gate& insert_gate( unsigned pos );

    /**
     * @brief Removes a gate at a given index
     *
     * If the index is not valid, no gate is removed.
     *
     * @param pos  Index
     *
     * @since  1.0
     */
    void remove_gate_at( unsigned pos );

    /**
     * @brief Sets the input names of the lines in a circuit
     *
     * This method sets the input names of the lines in a circuit.
     * This is useful for functions when writing them to a file,
     * printing them, or creating images.
     *
     * @param inputs Input names
     *
     * @since  1.0
     */
    void set_inputs( const std::vector<std::string>& inputs );

    /**
     * @brief Returns the input names of the lines in a circuit
     *
     * This method returns the input names of the lines in a circuit.
     * This is useful for functions when writing them to a file,
     * printing them, or creating images.
     *
     * @return Input names
     *
     * @since  1.0
     */
    const std::vector<std::string>& inputs() const;

    /**
     * @brief Sets the output names of the lines in a circuit
     *
     * This method sets the output names of the lines in a circuit.
     * This is useful for functions when writing them to a file,
     * printing them, or creating images.
     *
     * @param outputs Output names
     *
     * @since  1.0
     */
    void set_outputs( const std::vector<std::string>& outputs );

    /**
     * @brief Returns the output names of the lines in a circuit
     *
     * This method returns the output names of the lines in a circuit.
     * This is useful for functions when writing them to a file,
     * printing them, or creating images.
     *
     * @return Output names
     *
     * @since  1.0
     */
    const std::vector<std::string>& outputs() const;

    /**
     * @brief Sets the constant input line specifications
     *
     * This method sets the constant input line specification.
     *
     * Lines are by default not constant. If less values are given
     * than lines exist, the last ones will be not constant. If more
     * values are given than lines exist, they will be truncated.
     *
     * @sa constant
     *
     * @param constants Constant Lines
     *
     * @since  1.0
     */
    void set_constants( const std::vector<constant>& constants );

    /**
     * @brief Returns the constant input line specification
     *
     * This method returns the constant input line specification.
     *
     * @return Constant input line specification
     *
     * @since  1.0
     */
    const std::vector<constant>& constants() const;

    /**
     * @brief Sets whether outputs are garbage or not
     *
     * If an output is garbage it means, that the resulting
     * output value is not necessary for the function.
     *
     * Lines are by default not garbage. If less values are given
     * than lines exist, the last ones will be not garbage. If more
     * values are given than lines exist, they will be truncated.
     *
     * @param garbage Garbage line specification
     *
     * @since  1.0
     */
    void set_garbage( const std::vector<bool>& garbage );

    /**
     * @brief Returns whether outputs are garbage or not
     *
     * This method returns the garbage line specification.
     *
     * @return Garbage output line specification
     *
     * @since  1.0
     */
    const std::vector<bool>& garbage() const;

    /**
     * @brief Sets a name of the circuit
     *
     * Sets a name for the circuit which is empty
     * initially.
     *
     * @param name Name
     *
     * @since  1.0
     */
    void set_circuit_name( const std::string& name );

    /**
     * @brief Returns the name of the circuit
     *
     * Returns the name of the circuit which is empty
     * initially.
     *
     * @return Name of the circuit
     *
     * @since  1.0
     */
    const std::string& circuit_name() const;

    /**
     * @brief Constant access to the input buses
     *
     * This method gives constant access to the input
     * buses.
     *
     * @return Input bus collection
     *
     * @since  1.1
     */
    const bus_collection& inputbuses() const;

    /**
     * @brief Mutable access to the input buses
     *
     * This method gives mutable access to the input
     * buses.
     *
     * @return Input bus collection
     *
     * @since  1.1
     */
    bus_collection& inputbuses();

    /**
     * @brief Constant access to the output buses
     *
     * This method gives constant access to the output
     * buses.
     *
     * @return Output bus collection
     *
     * @since  1.1
     */
    const bus_collection& outputbuses() const;

    /**
     * @brief Mutable access to the output buses
     *
     * This method gives mutable access to the output
     * buses.
     *
     * @return Output bus collection
     *
     * @since  1.1
     */
    bus_collection& outputbuses();

    /**
     * @brief Constant access to the state signals
     *
     * This method gives constant access to the state
     * signals.
     *
     * @return State signal collection
     *
     * @since  1.1
     */
    const bus_collection& statesignals() const;

    /**
     * @brief Mutable access to the state signals
     *
     * This method gives mutable access to the state
     * signals.
     *
     * @return State signal collection
     *
     * @since  1.1
     */
    bus_collection& statesignals();

    /**
     * @brief Adds a module to the circuit
     *
     * This function adds a module to the circuit. It does
     * not create a gate calling the module, but the module
     * itself as a reference for further use, e.g. with append_module.
     *
     * This method uses smart pointer to a circuit wich already
     * exists and is managed by another object. If it cannot
     * be assured that the module is saved, the method
     * add_module(const std::string&, const circuit&) should be used.
     *
     * @param name Name of the module
     * @param module Reference to an existing module
     *
     * @since  1.1
     */
    void add_module( const std::string& name, const std::shared_ptr<circuit>& module );

    /**
     * @brief Adds a module to the circuit
     *
     * This function adds a module to the circuit. It does
     * not create a gate calling the module, but the module
     * itself as a reference for further use, e.g. with append_module.
     *
     * In this method the module is copied first and thus assured
     * that is managed by this circuit.
     *
     * @param name Name of the module
     * @param module Module to be copied
     *
     * @since  1.1
     */
    void add_module( const std::string& name, const circuit& module );

    /**
     * @brief Returns all modules in the circuit
     *
     * This method returns a map of all modules, whereby the
     * keys are the names of the modules.
     *
     * @return Map of modules
     *
     * @since  1.1
     */
    const std::map<std::string, std::shared_ptr<circuit> >& modules() const;

    /**
     * @brief Returns the annotation for one gate and one key
     *
     * This method returns the value for one particular annotation for
     * a given gate. If no annotation with that key exists, the a default
     * value is given.
     *
     * @param g Gate
     * @param key Key of the annotation
     * @param default_value Default value, in case the key does not exist
     *
     * @return Value of the annotation or the default value
     *
     * @since  1.1
     */
    const std::string& annotation( const gate& g, const std::string& key, const std::string& default_value = std::string() ) const;

    /**
     * @brief Returns all annotations for a given gate
     *
     * This method returns all annotations for a given gate. For the
     * purpose of efficiency, this method returns an optional data type
     * containing the property map. So, first check whether there are
     * items by assierting the optional, and then go through the map
     * by dereferencing the optional:
     * @code
     * boost::optional<const std::map<std::string, std::string>& > annotations = circ.annotations( g );
     *
     * if ( annotations )
     * {
     *   // annotations exists
     *   for ( const auto& p : *annotations )
     *   {
     *     const std::string& key = p.first;
     *     const std::string& value = p.second;
     *     // do something with key and value
     *   }
     * }
     * @endcode
     *
     * @param g Gate
     *
     * @return Map of annotations encapsulated in an optional
     *
     * @since  1.1
     */
    boost::optional<const std::map<std::string, std::string>& > annotations( const gate& g ) const;

    /**
     * @brief Annotates a gate
     *
     * With this method a gate can be annotated using a key and a value.
     * If there is an annotation with the same key, it will be overwritten.
     *
     * @param g Gate
     * @param key Key of the annotation
     * @param value Value of the annotation
     *
     * @since  1.1
     */
    void annotate( const gate& g, const std::string& key, const std::string& value );
  
  public:
    /** @cond */
    std::vector<std::shared_ptr<gate> > gates;
    unsigned _lines;

    std::vector<std::string> _inputs;
    std::vector<std::string> _outputs;
    std::vector<constant> _constants;
    std::vector<bool> _garbage;
    std::string _name;
    bus_collection _inputbuses;
    bus_collection _outputbuses;
    bus_collection _statesignals;
    std::map<const gate*, std::map<std::string, std::string> > _annotations;
  
    std::map<std::string, std::shared_ptr<circuit> > _modules;
    /** @endcond */
  };

}

#endif /* CIRCUIT_HPP */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
