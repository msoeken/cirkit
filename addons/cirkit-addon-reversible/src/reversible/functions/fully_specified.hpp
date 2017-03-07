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
 * @file fully_specified.hpp
 *
 * @brief Determines whether a truth_table is fully specified
 *
 * @author Mathias Soeken
 * @author Stefan Frehse
 * @since  1.0
 */

#ifndef FULLY_SPECIFIED_HPP
#define FULLY_SPECIFIED_HPP

#include <reversible/truth_table.hpp>

namespace cirkit
{

  /**
   * @brief Returns whether a truth table is fully specified
   *
   * This function checks whether a truth table \p tt is full specified,
   * that is input dimension is equal to output dimension and there is
   * no don't care value \p dc_value in any of the assigments.
   *
   * Further, the truth table cannot be empty.
   *
   * @note A fully specified truth table does not imply that it is reversible
   *
   * @param tt       Truth table
   * @param dc_value The don't care value for the truth table
   * @param is_reversible If true, number of outputs has to match number of inputs. Default value is \b true.
   * @return true, if it is fully speciefied, otherwise false
   *
   * @since  1.0
   */
  template<typename T>
  bool fully_specified( const truth_table<T>& tt, const typename truth_table<T>::value_type& dc_value, bool is_reversible = true )
  {
    if ( tt.num_inputs() == 0 )
    {
      return false;
    }

    if ( is_reversible && tt.num_inputs() != tt.num_outputs() )
    {
      return false;
    }

    if ( ( 1u << tt.num_inputs() ) != (unsigned)std::distance( tt.begin(), tt.end() ) )
    {
      return false;
    }

    for ( typename truth_table<T>::const_iterator it = tt.begin(); it != tt.end(); ++it )
    {
      if ( std::find( it->first.first, it->first.second, dc_value ) != it->first.second )
      {
        return false;
      }

      if ( std::find( it->second.first, it->second.second, dc_value ) != it->second.second )
      {
        return false;
      }
    }

    return true;
  }

  /**
   * @brief Returns whether a truth table is fully specified
   *
   * This function is specialized for a reversible_truth_table,
   * but calls the generic version with the don't care value.
   *
   * @param tt       Truth table
   * @param is_reversible If true, number of outputs has to match number of inputs. Default value is \b true.
   * @return true, if it is fully speciefied, otherwise false
   *
   * @since  1.0
   */
  bool fully_specified( const binary_truth_table& tt, bool is_reversible = true );

}

#endif /* FULLY_SPECIFIED_HPP */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
