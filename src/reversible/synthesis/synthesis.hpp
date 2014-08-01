/* RevKit (www.rekit.org)
 * Copyright (C) 2009-2014  University of Bremen
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
// End:
