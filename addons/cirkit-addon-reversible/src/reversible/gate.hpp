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
 * @file gate.hpp
 *
 * @brief Gate class
 *
 * @author Mathias Soeken
 * @author Stefan Frehse
 * @since  1.0
 */

#ifndef GATE_HPP
#define GATE_HPP

#include <reversible/variable.hpp>

#include <iostream>
#include <set>
#include <vector>

#include <boost/any.hpp>

namespace cirkit
{

  /**
   * @brief Represents a gate in a circuit
   *
   * @since  1.0
   */
  class gate
  {
  public:
    /**
     * @brief Vector type of gates
     * @since  1.0
     */
    using vector = std::vector<gate>;

    /**
     * @brief Container for storing control lines
     * @since  2.0
     */
    using control_container = std::vector<variable>;

    /**
     * @brief Container for storing target lines
     * @since 2.0
     */
    using target_container = std::vector<unsigned>;

  public:
    /**
     * @brief Default constructor
     *
     * Initializes private data
     *
     * @since  1.0
     */
    gate();

    /**
     * @brief Copy Constructor
     *
     * Initializes private data and copies gate
     *
     * @param other Gate to be assigned
     *
     * @since  1.0
     */
    gate( const gate& other );

    /**
     * @brief Default deconstructor
     *
     * Clears private data
     *
     * @since  1.0
     */
    virtual ~gate();

    /**
     * @brief Assignment operator
     *
     * @param other Gate to be assigned
     *
     * @return Pointer to instance
     *
     * @since  1.0
     */
    gate& operator=( const gate& other );

    /**
     * @brief Returns the control lines
     *
     * @since  2.0
     */
    gate::control_container& controls() const;

    /**
     * @brief Returns the target lines
     *
     * @since  2.0
     */
    gate::target_container& targets() const;

    /**
     * @brief Returns the number of control and target lines as sum
     *
     * This method returns the number of control and target
     * lines as sum and can be used for e.g. calculating costs.
     *
     * @since  1.0
     *
     * @return Number of control and target lines.
     */
    virtual unsigned size() const;

    /**
     * @brief Adds a control line to the gate
     *
     * @param c control variable to add
     *
     * @since 1.0
     */
    virtual void add_control( variable c );

    /**
     * @brief Remove control line to the gate
     *
     * @param c control variable to remove
     *
     * @since 1.0
     */
    virtual void remove_control( variable c );

    /**
     * @brief Adds a target to the desired line
     *
     * @param l target line
     *
     * @since 1.0
     */
    virtual void add_target( unsigned l );

    /**
     * @brief Removes a target from the desired line
     *
     * @param l target line
     *
     * @since 1.0
     */
    virtual void remove_target( unsigned l );

    /**
     * @brief Sets the type of the target line(s)
     *
     * @param t target type
     *
     * @since  1.0
     */
    virtual void set_type( const boost::any& t );

    /**
     * @brief Returns the type of the target line(s)
     *
     * @return target type
     *
     * @since  1.0
     */
    virtual const boost::any& type() const;

  private:
    class priv;
    priv* const d;
  };
}

#endif /* GATE_HPP */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
