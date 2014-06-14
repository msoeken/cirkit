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
 * @file rcbdd_synthesis.hpp
 *
 * @brief Synthesis based on RCBDDs
 *
 * @author Mathias Soeken
 * @author Laura Tague
 * @since  2.0
 */

#ifndef RCBDD_SYNTHESIS_HPP
#define RCBDD_SYNTHESIS_HPP

#include <string>

#include <reversible/rcbdd.hpp>
#include <reversible/synthesis/synthesis.hpp>

namespace cirkit
{

  /**
   * @brief Embedding of an irreversible specification
   *
   * @since  2.0
   */
  bool rcbdd_synthesis( circuit& circ, const rcbdd& cf,
                        properties::ptr settings = properties::ptr(),
                        properties::ptr statistics = properties::ptr() );

}

#endif /* RCBDD_SYNTHESIS_HPP */

// Local Variables:
// c-basic-offset: 2
// End:
