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

  bool operator==( const assignment_t& other ) const
  {
    return ( bits == other.bits && mask == other.mask );
  }
  
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
