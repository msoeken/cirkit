/* RevKit: A Toolkit for Reversible Circuit Design (www.revkit.org)
 * Copyright (C) 2009-2014  The RevKit Developers <revkit@informatik.uni-bremen.de>
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
// End:
