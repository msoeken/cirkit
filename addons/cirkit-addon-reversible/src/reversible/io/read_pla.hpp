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
 * @file read_pla.hpp
 *
 * @brief Reads a specification from a PLA file
 *
 * @author Mathias Soeken
 * @since  1.0
 */

#ifndef READ_PLA_HPP
#define READ_PLA_HPP

#include <reversible/truth_table.hpp>

namespace cirkit
{

  /**
   * @brief Settings for read_pla function
   *
   * @since  1.0
   */
  struct read_pla_settings
  {
    /**
     * @brief If true, the truth table is extended after parsing
     *
     * If this variable is set to true, then 1) all cubes
     * like e.g. -01- 1 get extended, so that four cubes
     * are added, which are 0010 1, 0011 1, 1010 1, and 1011 1.
     * That is, all possible Boolean combinations for the don't
     * care values are assigned. Further some PLA implicitly assume
     * that cubes which are not mentioned return 0 as output values
     * for all specified functions. These cubes are added as well.
     *
     * Default value is \b true.
     *
     * @since  1.0
     */
    bool extend = true;

    /**
     * @since  2.0
     */
    bool skip_after_first_cube = false;
  };

  binary_truth_table::cube_type combine_pla_cube( const binary_truth_table::cube_type& c1, const binary_truth_table::cube_type& c2 );

  /**
   * @brief Reads a specification from a PLA file
   *
   * This function parses an PLA file and creates a truth table.
   * Thereby only the specified cubes of the PLA are added as entries including the don't care values.
   * For extending the truth table, i.e. filling the don't cares and specifying the 0-outputs explicitly, call extend_truth_table.
   *
   * @param spec The truth table
   * @param filename File-name to read PLA from
   * @param settings Settings for read_pla
   * @param error If not 0, an error message is assigned when the function returns false
   * @return true on success
   *
   * @since  1.0
   */
  bool read_pla( binary_truth_table& spec, const std::string& filename, const read_pla_settings& settings = read_pla_settings(), std::string* error = 0 );

  /**
   * @brief Reads only the size of the PLA, i.e. inputs and outputs without
   *        parsing the whole file.
   *
   * @since  2.0
   */
  std::pair<unsigned, unsigned> read_pla_size( const std::string& filename );

}

#endif /* READ_PLA_HPP */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
