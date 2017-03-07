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
 * @file pattern.hpp
 *
 * @brief Data Structure for Simulation Pattern
 *
 * @author Mathias Soeken
 * @since  1.2
 */

#include <map>
#include <string>
#include <vector>

namespace cirkit
{

  /**
   * @brief Pattern file for sequential simulation
   *
   * This class is used together with read_pattern and create_simulation_pattern
   * to create simulation pattern for sequential_simulation.
   *
   * @code
   * circuit circ;
   * // create circuit somehow
   *
   * pattern p;
   * read_pattern( p, "pattern.sim" );
   *
   * std::vector<boost::dynamic_bitset<> > sim;
   * std::map<std::string, boost::dynamic_bitset<> > init;
   * create_simulation_pattern( p, circ, sim, init );
   *
   * properties::ptr settings( new properties() );
   * settings->set( "initial_state", init );
   *
   * std::vector<boost::dynamic_bitset<> > outputs;
   * sequential_simulation( outputs, circ, sim, settings );
   * @endcode
   *
   * @since  1.2
   */
  class pattern
  {
  public:
    /**
     * @brief Standard constructor
     *
     * @since  1.2
     */
    pattern();

    /**
     * @brief Deconstructor
     *
     * @since  1.2
     */
    ~pattern();

    /**
     * @brief Map for initializers
     *
     * The key is the name of the state signal and
     * the value is the assigned initial value.
     *
     * @since  1.2
     */
    using initializer_map = std::map<std::string, unsigned>;

    /**
     * @brief Pattern Type
     *
     * A pattern is a sequence of values
     * which are assigned to the input signals
     * at every step of simulation.
     *
     * @since  1.2
     */
    using pattern_t = std::vector<unsigned>;

    /**
     * @brief Vector of pattern
     *
     * @since  1.2
     */
    using pattern_vec = std::vector<pattern_t>;

    /**
     * @brief Adds an initializer
     *
     * @param name Name of the state signal
     * @param value Initial value
     *
     * @since  1.2
     */
    void add_initializer( const std::string& name, unsigned value );

    /**
     * @brief Adds an input signal
     *
     * @param name Name of the input signal
     *
     * @since  1.2
     */
    void add_input( const std::string& name );

    /**
     * @brief Adds a pattern sequence
     *
     * The pattern sequence must have the size of all
     * specified inputs.
     *
     * @param pattern Vector of pattern sequence
     *
     * @since  1.2
     */
    void add_pattern( const std::vector<unsigned>& pattern );

    /**
     * @brief Returns the initializers
     *
     * A map is returned with a the name of the
     * state signal as key and the initial value
     * as key.
     *
     * @return Map of initializers
     */
    const initializer_map& initializers() const;

    /**
     * @brief Returns the list of input signals
     *
     * @return List of input signals
     */
    const std::vector<std::string>& inputs() const;

    /**
     * @brief Returns the list of pattern sequences
     *
     * @return List of pattern sequences
     */
    const pattern_vec& patterns() const;

  private:
    class priv;
    priv* const d;
  };

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
