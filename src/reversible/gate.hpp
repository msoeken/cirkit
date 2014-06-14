/* RevKit: A Toolkit for Reversible Circuit Design (www.revkit.org)
 * Copyright (C) 2009-2013  The RevKit Developers <revkit@informatik.uni-bremen.de>
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
    typedef std::vector<gate>  vector;

    /**
     * @brief Container for storing control lines
     * @since  2.0
     */
    typedef std::vector<variable> control_container;

    /**
     * @brief Container for storing target lines
     * @since 2.0
     */
    typedef std::vector<unsigned> target_container;

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
    gate::control_container controls() const;

    /**
     * @brief Returns the target lines
     *
     * @since  2.0
     */
    gate::target_container targets() const;

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
// End:
