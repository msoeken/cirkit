/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2016  EPFL
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
