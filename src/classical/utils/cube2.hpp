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
 * @file cube2.hpp
 *
 * @brief A (new) efficient cube data structure for up to 32 inputs
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef CUBE2_HPP
#define CUBE2_HPP

#include <array>
#include <cstdint>
#include <iostream>

namespace cirkit
{

class cube2
{
public:
  /* constructors */
  cube2();
  cube2( uint32_t bits, uint32_t mask );

  /* query operations (unary) */
  int num_literals() const;

  /* query operations (binary) */
  int distance( const cube2& that ) const;
  uint32_t differences( const cube2& that ) const;
  bool operator==( const cube2& that ) const;
  bool operator!=( const cube2& that ) const;

  /* operators (binary) */
  cube2 operator&( const cube2& that ) const;
  cube2 merge( const cube2& that ) const;
  std::array<cube2, 4> exorlink( const cube2& that, int distance, uint32_t differences, unsigned* group ) const;

  /* modify operations */
  void invert_all();
  void rotate( unsigned bit ); /* x -> ~x -> * -> x -> ... */

  //
  // bm  bm'
  // 11  01 *
  // 01  00  *
  // 00  11 **
  //
  // b changes if b <-> m
  // m changes if !b

  /* construction */
  static cube2 one_cube();
  static cube2 zero_cube();
  static cube2 elementary_cube( unsigned index );

  /* printing / debugging */
  void print( unsigned length = 32u, std::ostream& os = std::cout ) const;



  /* cube data */
  union
  {
    struct
    {
      uint32_t bits;
      uint32_t mask;
    };
    uint64_t value;
  };
};

std::ostream& operator<<( std::ostream& os, const cube2& cube );

}

namespace std
{

template<>
struct hash<cirkit::cube2>
{
  std::size_t operator()( cirkit::cube2 const& c ) const
  {
    return c.value;
  }
};

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
