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
 * @file transposition_based_synthesis.hpp
 *
 * @brief A simple synthesis algorithm based on transpositions
 *
 * @author Mathias Soeken
 * @since  1.3
 */
#ifndef TRANSPOSITION_BASED_SYNTHESIS
#define TRANSPOSITION_BASED_SYNTHESIS

#include <core/properties.hpp>

#include <reversible/circuit.hpp>
#include <reversible/truth_table.hpp>
#include <boost/dynamic_bitset.hpp>
#include <reversible/synthesis/synthesis.hpp>

namespace cirkit
{

  /**
   * @brief A simple synthesis algorithm based on transpositions
   *
   * @since  1.3
   */
  bool transposition_based_synthesis( circuit& circ, const binary_truth_table& spec, properties::ptr settings = properties::ptr(), properties::ptr statistics = properties::ptr() );

  /**
   * @brief Functor for the transposition_based_synthesis algorithm
   *
   * @param settings Settings (see transposition_based_synthesis)
   * @param statistics Statistics (see transposition_based_synthesis)
   *
   * @return A functor which complies with the truth_table_based_synthesis_func interface
   *
   * @since  1.3
   */
  truth_table_synthesis_func transposition_based_synthesis_func( properties::ptr settings = std::make_shared<properties>(), properties::ptr statistics = std::make_shared<properties>() );

}

#endif /* TRANSPOSITION_BASED_SYNTHESIS */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
