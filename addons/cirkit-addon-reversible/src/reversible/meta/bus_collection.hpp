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
 * @file bus_collection.hpp
 *
 * @brief Bus Collection
 *
 * @author Mathias Soeken
 * @since  1.1
 */

#ifndef BUS_COLLECTION_HPP
#define BUS_COLLECTION_HPP

#include <map>
#include <string>
#include <vector>

#include <boost/optional.hpp>

namespace cirkit
{

  /**
   * @brief Collection for buses
   *
   * This class represents a collection of buses.
   * Using respective methods it is possible to add
   * new buses or find buses by name or line index.
   *
   * Buses are usually accessed via the methods circuit::inputbuses()
   * and circuit::outputbuses()
   *
   * @since  1.1
   */
  class bus_collection
  {
  public:
    /**
     * @brief
     *
     * @since  1.1
     */
    typedef std::map<std::string, std::vector<unsigned> > map;

    /**
     * @brief Standard constructor
     *
     * @since  1.1
     */
    bus_collection();

    /**
     * @brief Standard copy constructor
     *
     * @since  2.0
     */
    bus_collection( const bus_collection& other );

    /**
     * @brief Deconstructor
     *
     * @since  1.1
     */
    ~bus_collection();

    /**
     * @brief Standard assignment operator
     *
     * @since  2.0
     */
    bus_collection& operator=( const bus_collection& other );

    /**
     * @brief Adds a new bus to the collection
     *
     * @param name Name of the bus
     * @param line_indices Corresponding lines of the bus
     * @param initial_value This value can optional be set
     *                      to assign an initial value to
     *                      this bus.
     *
     * @since  1.1
     */
    void add( const std::string& name, const std::vector<unsigned>& line_indices, const boost::optional<unsigned>& initial_value = boost::optional<unsigned>() );

    /**
     * @brief Gets the corresponding lines of a bus by the name
     *
     * If there is no such bus with the name \p name in the collection,
     * an assertion is thrown. This method is meant to be used in conjunction
     * with find_bus.
     *
     * @param name Name of the bus
     *
     * @return Corresponding lines of the bus
     *
     * @since  1.1
     */
    const std::vector<unsigned>& get( const std::string& name ) const;

    /**
     * @brief Returns all buses of the collection
     *
     * This method returns all the buses of the collection.
     *
     * @return All buses of the collection
     *
     * @since  1.1
     */
    const map& buses() const;

    /**
     * @brief This method finds the bus for a line
     *
     * If the line belongs to a bus, the name of the bus
     * is returned, otherwise an empty string is returned.
     *
     * If the name is not important, but just the check whether the
     * line is contained in some bus, use this method rather than bus_collection::find_bus
     *
     * @param line_index Index of the line
     *
     * @return Name of the bus, or empty string in case the line does not belong to any bus
     *
     * @since  1.1
     */
    std::string find_bus( unsigned line_index ) const;

    /**
     * @brief This method determines whether there exists a bus for a given line
     *
     * If the line at \p line_index is contained in a bus, this
     * method returns \b true, otherwise \b false.
     *
     * If the name is not important, but just the check whether the
     * line is contained in some bus, use this method rather than bus_collection::find_bus
     *
     * @param line_index Index of the line
     *
     * @return \b true, if line at \p line_index is contained in a bus
     */
    bool has_bus( unsigned line_index ) const;

    /**
     * @brief Returns the signal index relative to the bus
     *
     * If e.g. a bus \b A is defined by the line indices 3,4,6
     * then the signal index of 4 is 1, since it is the 2nd signal
     * in the bus (considering counting from 0).
     *
     * This method requires, that \p line_index belongs to a bus,
     * otherwise an assertion is thrown.
     *
     * @param line_index Index of the line
     *
     * @return Index of the signal relative to the bus
     *
     * @since  1.1
     */
    unsigned signal_index( unsigned line_index ) const;

    /**
     * @brief Sets the initial value of a bus
     *
     * This method is used primarily for state signals, which
     * can be assigned an initial value. This method is called
     * with the name of the bus. If no such bus exists, this method
     * call has no effect.
     *
     * @param name Name of the bus
     * @param initial_value Initial value
     *
     * @since  1.1
     */
    void set_initial_value( const std::string& name, unsigned initial_value );

    /**
     * @brief Retrieves the initial value of a bus
     *
     * Given a name of the bus, this method tries to retrieve an initial
     * value. If no bus with this name exists, or if a bus exist but
     * does not have an initial value, an empty optional is returned.
     * Thus, this method should be used as demonstrated in the following example:
     * @code
     * boost::optional<unsigned> iv = bus.initial_value( "bus_name" );
     * if ( iv )
     * {
     *   // bus has the initial value *iv
     *   std::cout << "Initial value: " << iv << std::endl;
     * }
     * @endcode
     * If there exists a default value for the initial value, the following
     * shorter snippet can be used:
     * @code
     * unsigned iv = bus.initial_value( "bus_name" ).get_value_or( 0u );
     * @endcode
     *
     * @param name Name of the bus
     *
     * @return Initial value
     *
     * @since  1.1
     */
    boost::optional<unsigned> initial_value( const std::string& name ) const;

  private:
    class priv;
    priv* const d;
  };

}

#endif /* BUS_HPP */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
