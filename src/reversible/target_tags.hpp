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
 * @file target_tags.hpp
 *
 * @brief Predefined target type tags for common gate types
 *
 * @sa \ref sub_target_tags
 *
 * @author Mathias Soeken
 * @since  1.0
 */

#ifndef TARGET_TAGS_HPP
#define TARGET_TAGS_HPP

#include <reversible/circuit.hpp>
#include <reversible/gate.hpp>

#include <boost/any.hpp>

namespace cirkit
{

  /**
   * @brief Target Tag for Toffoli gates.
   *
   * @sa \ref sub_target_tags
   *
   * @since  1.0
   */
  struct toffoli_tag {};

  /**
   * @brief Target Tag for Fredkin gates.
   *
   * @sa \ref sub_target_tags
   *
   * @since  1.0
   */
  struct fredkin_tag {};

  /**
   * @brief Target Tag for Peres gates.
   *
   * @sa \ref sub_target_tags
   *
   * @since  1.0
   */
  struct peres_tag {};

  /**
   * @brief Target Tag for Modules
   *
   * @sa \ref sub_target_tags
   *
   * @since  1.1
   */
  struct module_tag
  {
    /**
     * @brief Name of the module
     *
     * @since  1.1
     */
    std::string name;

    /**
     * @brief Reference to the circuit
     *
     * Usually the circuit is inside of the
     * circuit modules list.
     *
     * @since  1.1
     */
    std::shared_ptr<circuit> reference;
  };

  /**
   * @brief Compares type of a boost::any variable
   *
   * This method is called by is_\em gate functions
   * like is_toffoli().
   *
   * @param operand A variable of type boost::any
   * @return true, if \p operand is of type \p T.
   *
   * @since  1.0
   */
  template<typename T>
  bool is_type( const boost::any& operand )
  {
    return operand.type() == typeid( T );
  }

  /**
   * @brief Checks if two gates have the same type
   *
   * Use this function, since == does not work on gate::type
   * to compare to gates by its type.
   *
   * @param g1 First gate
   * @param g2 Second gate
   * @return true, if they have the same target tag, otherwise false
   *
   * @since  1.0
   */
  bool same_type( const gate& g1, const gate& g2 );

  /**
   * @brief Returns whether a gate is a Toffoli gate
   *
   * @param g Gate
   * @return true, if \p g is a Toffoli gate
   *
   * @since  1.0
   */
  bool is_toffoli( const gate& g );

  /**
   * @brief Returns whether a gate is a Fredkin gate
   *
   * @param g Gate
   * @return true, if \p g is a Fredkin gate
   *
   * @since  1.0
   */
  bool is_fredkin( const gate& g );

  /**
   * @brief Returns whether a gate is a Peres gate
   *
   * @param g Gate
   * @return true, if \p g is a Peres gate
   *
   * @since  1.0
   */
  bool is_peres( const gate& g );

  /**
   * @brief Returns whether a gate is a module
   *
   * @param g Gate
   * @return true, if \p g is a module
   *
   * @since  1.1
   */
  bool is_module( const gate& g );

}

#endif /* TARGET_TAGS_HPP */

// Local Variables:
// c-basic-offset: 2
// End:
