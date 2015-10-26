/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
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
 * @file counterexample.hpp
 *
 * @brief Counterexample
 *
 * @author Heinz Riener
 * @since  2.3
 */

#ifndef COUNTEREXAMPLE_HPP
#define COUNTEREXAMPLE_HPP

#include <boost/dynamic_bitset.hpp>
#include <string>
#include <ostream>

namespace cirkit
{

struct assignment_t
{
  assignment_t()
  {}

  assignment_t( const unsigned size )
    : bits( boost::dynamic_bitset<>( size ) )
    , mask( boost::dynamic_bitset<>( size ) )
  {}

  assignment_t( boost::dynamic_bitset<> bits, boost::dynamic_bitset<> mask )
    : bits( bits )
    , mask( mask )
  {
    assert( bits.size() == mask.size() );
  }

  assignment_t( const std::string& assignment )
    : bits( assignment )
    , mask( std::string( assignment.size(), '1' ) )
  {}

  unsigned size() const
  {
    assert( bits.size() == mask.size() );
    return bits.size();
  }

  boost::dynamic_bitset<> bits;
  boost::dynamic_bitset<> mask;
}; /* assignment_t */

std::ostream& operator<<( std::ostream& os, const assignment_t& a );

struct counterexample_t
{
  counterexample_t()
  {}

  counterexample_t( const std::size_t num_inputs, const std::size_t num_outputs )
    : in( assignment_t( num_inputs ) )
    , out( assignment_t( num_outputs ) )
    , expected_out( assignment_t( num_outputs ) )
  {}

  inline bool empty() const
  {
    return ( in.size() == 0u && out.size() == 0u && expected_out.size() == 0u );
  }

  inline operator bool() const
  {
    return !empty();
  }

  assignment_t in;
  assignment_t out;
  assignment_t expected_out;
}; /* counterexample_t */

std::ostream& operator<<( std::ostream& os, const counterexample_t& cex );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
