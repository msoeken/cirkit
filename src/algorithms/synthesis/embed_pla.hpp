/* RevKit: A Toolkit for Reversible Circuit Design (www.revkit.org)
 * Copyright (C) 2009-2013  The RevKit Developers <revkit@informatik.uni-bremen.de>
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
 * @file embed_pla.hpp
 *
 * @brief Embedding of an irreversible specification given as PLA
 *
 * @author Mathias Soeken
 * @since  2.0
 */

#ifndef EMBED_PLA_HPP
#define EMBED_PLA_HPP

#include <string>

#include <core/rcbdd.hpp>
#include <algorithms/synthesis/synthesis.hpp>

namespace revkit
{

  /**
   * @brief Embedding of an irreversible specification
   *
   * @since  2.0
   */
  bool embed_pla( rcbdd& cf, const std::string& filename,
                  properties::ptr settings = properties::ptr(),
                  properties::ptr statistics = properties::ptr() );

  /**
   * @brief Functor for the \ref revkit::embed_pla "embed_pla" algorithm
   *
   * @param settings Settings (see \ref revkit::embed_pla "embed_pla")
   * @param statistics Statistics (see \ref revkit::embed_pla "embed_pla")
   *
   * @return A functor
   *
   * @since  1.0
   */
  //embedding_func embed_truth_table_func( properties::ptr settings = properties::ptr( new properties() ), properties::ptr statistics = properties::ptr( new properties() ) );

}

#endif /* EMBED_PLA_HPP */

// Local Variables:
// c-basic-offset: 2
// End:
