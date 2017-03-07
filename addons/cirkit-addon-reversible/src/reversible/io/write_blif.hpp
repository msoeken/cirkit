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
 * @file write_blif.hpp
 *
 * @brief Writes a circuit to a BLIF file
 *
 * @author Mathias Soeken
 * @since  1.0
 */

#ifndef WRITE_BLIF_HPP
#define WRITE_BLIF_HPP

#include <iostream>
#include <map>
#include <vector>

#include <boost/optional.hpp>

namespace cirkit
{

  class circuit;
  class gate;

  /**
   * @brief Settings for write_blif
   *
   * @since  1.0
   */
  struct write_blif_settings
  {
    /**
     * @brief Stores truth tables
     *
     * The key is the index of the respective target line.
     * The value is a map itself, which maps input monoms
     * to output values for that target.
     *
     * @since  1.1.1
     */
    using truth_table_map = std::map<unsigned, std::map<std::vector<boost::optional<bool> >, bool> >;

    /**
     * @brief Prefix for the auxiliary variables which are created by the algorithm
     *
     * Default value is \b tmp
     *
     * @since  1.0
     */
    std::string tmp_signal_name = "tmp";

    /**
     * @brief Sets if output should comply to BlifMV
     *
     * In BlifMV spaces are inserted between the elements
     * of the input cubes.
     *
     * Default value is \b false
     *
     * @since  1.2
     */
    bool blif_mv = false;

    /**
     * @brief Output prefix for output signals
     *
     * @since 2.0
     */
    std::string output_prefix;

    /**
     * @brief Sets an state prefix for output signals
     *
     * Since states have both the same input and output name
     * this prefix is prepended to the respective output names.
     *
     * Default value is \b out_
     *
     * @since  1.3
     */
    std::string state_prefix = "out_";

    /**
     * @brief Sets if constant signals should keep their name
     *
     * Keeps constant names, but only if they are unique.
     *
     * Default value is \b false
     *
     * @since  1.3
     */
    bool keep_constant_names = false;

    /**
     * @brief Operator for transforming the gates into BLIF code
     *
     * By convention the first input signals are for the target lines and then the control lines.
     * The number of output signals is the number of target lines.
     * Only the cubes have to be printed, not the \em .names declaration.
     *
     * This operator has to be overridden when new gate types should be supported.
     *
     * The signature of this operator changed in RevKit version 1.1.1
     *
     * @param g The current g to be transformed
     * @param map Truth table map to be filled with cubes (check the source code for examples) (since 1.1.1)
     *
     * @since  1.0
     */
    virtual void operator()( const gate& g, truth_table_map& map ) const;

  };

  /**
   * @brief Writes a circuit to a BLIF file
   *
   * This function writes the circuit to a BLIF file.
   * Therefore, it is required that the circuit has names for all its inputs and outputs.
   *
   * @param circ Circuit
   * @param os Output stream where to write the result to. Default is \b STDOUT.
   * @param settings Settings
   *
   * @since  1.0
   */
  void write_blif( const circuit& circ, std::ostream& os = std::cout, const write_blif_settings& settings = write_blif_settings() );

}

#endif /* WRITE_BLIF_HPP */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
