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
 * @file read_pla_to_bdd.hpp
 *
 * @brief Reads a BDD from a PLA file
 *
 * @author Mathias Soeken
 * @since  1.3
 */

#ifndef READ_PLA_TO_BDD_HPP
#define READ_PLA_TO_BDD_HPP

#include <functional>
#include <iostream>
#include <map>
#include <vector>

#include <boost/optional.hpp>

#include <cudd.h>

#include <core/properties.hpp>

namespace cirkit
{

  /**
   * @brief Contains the result data for read_pla_to_bdd
   *
   * @since  1.3
   */
  class BDDTable
  {
  public:
    /**
     * @brief Default constructor
     *
     * @since  1.3
     */
    BDDTable();

    /**
     * @brief Use existing manager
     *
     * @since  2.0
     */
    BDDTable( DdManager * manager );

    /**
     * @brief Deconstructor
     *
     * @since  1.3
     */
    ~BDDTable();

    /**
     * @brief Stores the input nodes (with name)
     */
    std::vector<std::pair<std::string, DdNode*> > inputs;

    /**
     * @brief Stores the output nodes (with name)
     */
    std::vector<std::pair<std::string, DdNode*> > outputs;

    /**
     * @brief If the characteristic function is generated (not used so far),
     *        then the real number of inputs is stored here as outputs will
     *        always contain one element in this case.
     */
    boost::optional<unsigned> num_real_outputs;

    /**
     * @brief The internal CUDD manager object
     */
    DdManager* cudd;

  private:
    bool external_manager = false;
  };

  using generation_func_type = std::function<DdNode*(DdManager*, unsigned)>;

  /**
   * @brief Reads a BDD from a PLA file
   *
   * @since  1.3
   */
  bool read_pla_to_bdd( BDDTable& bdd, const std::string& filename,
                        const properties::ptr& settings = properties::ptr(),
                        const properties::ptr& statistics = properties::ptr() );

  /**
   * @brief Reads a characteristic BDD from a PLA file
   *
   * @param bdd Table to store the BDD nodes
   * @param filename PLA filename
   * @param inputs_first Variable ordering, either inputs before outputs or the other way around
   * @param with_output_zero_patterns Determines whether the 0...0 output pattern should explicitly be considered
   *
   * @since  2.0
   */
  bool read_pla_to_characteristic_bdd( BDDTable& bdd, const std::string& filename, bool inputs_first = true, bool with_output_zero_patterns = false );

}

#endif /* READ_PLA_TO_BDD_HPP */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
