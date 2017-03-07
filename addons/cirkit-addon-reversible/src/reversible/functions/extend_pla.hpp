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
 * @file extend_pla.hpp
 *
 * @brief Extends a PLA
 *
 * @author Mathias Soeken
 * @since  2.0
 */

#include <reversible/truth_table.hpp>

#ifndef EXTEND_PLA_HPP
#define EXTEND_PLA_HPP

namespace cirkit
{

  /**
   * @brief Settings for extend_pla_settings
   *
   * @since  2.0
   */
  struct extend_pla_settings
  {
    /**
     * @brief Compact extended PLA after creation
     *
     * If this settings is set to true (which is default)
     * the extended PLA is compacted in a post process.
     * Although, the PLA becomes significantly smaller,
     * it may still be exponential in the worst case before this
     * step.
     *
     * @since  2.0
     */
    bool post_compact = true;

    /**
     * @brief Be verbose
     *
     * @since  2.0
     */
    bool verbose = false;
  };

  /**
   * @brief Extends a PLA representation such that it does
   *        not contain any intersecting input cubes.
   *
   * Caution: Note that this function changes the input parameter base.
   *
   * @param base     The original PLA representation
   * @param extended The extended new PLA representation
   * @param settings Settings
   *
   * @since  2.0
   */
  void extend_pla( binary_truth_table& base, binary_truth_table& extended, const extend_pla_settings& settings = extend_pla_settings() );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
