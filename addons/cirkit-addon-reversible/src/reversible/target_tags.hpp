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
#include <boost/dynamic_bitset.hpp>

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
 * @brief Single target tag
 *
 * @sa \ref sub_target_tags
 *
 * @since 2.3
 */
struct stg_tag
{
  stg_tag() {}
  stg_tag( const boost::dynamic_bitset<>& function ) : function( function ) {}

  boost::dynamic_bitset<> function;
  boost::dynamic_bitset<> affine_class;
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

/**
 * @brief Returns whether a gate is a single-target gate
 *
 * @param g Gate
 * @return true, if \p g is a single-target gate
 *
 * This function does not return true, e.g., if \p g is a Toffoli gate
 * even though a Toffoli gate can be considered a specialization of a
 * single-target gate
 *
 * @since  2.3
 */
bool is_stg( const gate& g );

}

#endif /* TARGET_TAGS_HPP */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
