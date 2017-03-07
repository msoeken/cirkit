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
 * @file synthesis.hpp
 *
 * @brief General Synthesis type definitions
 *
 * @author Mathias Soeken
 * @since  1.0
 */

#ifndef SYNTHESIS_HPP
#define SYNTHESIS_HPP

#include <string>

#include <boost/function.hpp>

#include <core/functor.hpp>

#include <reversible/circuit.hpp>
#include <reversible/rcbdd.hpp>
#include <reversible/truth_table.hpp>

namespace cirkit
{

  /**
   * @brief Functor for synthesis based on a truth table
   *
   * @since  1.0
   */
  typedef functor<bool(circuit&, const binary_truth_table&)> truth_table_synthesis_func;

  /**
   * @brief Functor for synthesis based on a file-name (PLA or BLIF)
   *
   * @since  1.0
   */
  typedef functor<bool(circuit&, const std::string&)> pla_blif_synthesis_func;

  /**
   * @brief Functor for embedding a binary truth table in place
   *
   * @since  1.0
   */
  typedef functor<bool(binary_truth_table&, const binary_truth_table&)> embedding_func;

  /**
   * @brief Functor for embedding a PLA to a RCBDD
   *
   * @since  2.0
   */
  typedef functor<bool(rcbdd&, const std::string&)> pla_embedding_func;

  /**
   * @brief Functor for decomposing a reversible circuit into a quantum circuit
   *
   * @since  1.0
   */
  typedef functor<bool(circuit&, const circuit&)> decomposition_func;
}

#endif /* SYNTHESIS_HPP */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
