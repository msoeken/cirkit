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
 * @file spec_representation.hpp
 *
 * @brief Generic specification interface
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef SPEC_REPRESENTATION_HPP
#define SPEC_REPRESENTATION_HPP

#include <vector>

#include <boost/dynamic_bitset.hpp>
#include <boost/optional.hpp>
#include <boost/variant.hpp>

#include <classical/mig/mig.hpp>
#include <classical/utils/truth_table_utils.hpp>

namespace cirkit
{

class spec_representation
{
public:
  using spec_t = boost::variant<tt, mig_graph>;

  spec_representation( const spec_t& spec );

  /**
   * @return true if and only if spec is truth table
   */
  bool is_explicit() const;

  /**
   * @return number of variables
   */
  unsigned num_vars() const;

  /**
   * @return support
   *
   * bit is i if x_i is in the (structural) support
   */
  boost::dynamic_bitset<> support() const;

  /**
   * @return pairs of symmetric variables
   *
   * in a pair (i,j) we always have i < j
   */
  std::vector<std::pair<unsigned, unsigned>> symmetric_variables() const;

  /**
   * @return whether the spec represents a constant or single-variable function
   *
   * returns (0, compl) if spec is compl
   * returns (i, compl) if spec is x_i ^ compl (or technically x_{i-1})
   */
  boost::optional<std::pair<unsigned, bool>> is_trivial() const;

  /**
   * @return true if f(0, ..., 0) = 0
   */
  bool is_normal() const;

  void invert();

  template<typename Visitor>
  typename Visitor::result_type apply_visitor( Visitor& visitor ) const
  {
    return boost::apply_visitor( visitor, spec );
  }

  template<typename Visitor>
  typename Visitor::result_type apply_visitor( const Visitor& visitor ) const
  {
    return boost::apply_visitor( visitor, spec );
  }

private:
  spec_t spec;
};

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
