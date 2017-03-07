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

#include "synthesis_utils_p.hpp"

namespace cirkit
{

  unsigned hamming_distance( unsigned a, unsigned b )
  {
    unsigned x = a ^ b;
    unsigned bits = 0;

    while ( x )
    {
      x &= ( x - 1 );
      ++bits;
    }

    return bits;
  }

  mpz_class pow2(unsigned n)
  {
    return mpz_class("1" + std::string(n, '0'), 2);
  }

  unsigned calculate_required_lines(unsigned n, unsigned m, mpz_class maxmu)
  {
    unsigned exp = 0u;

    while (pow2(exp) < maxmu) {
      ++exp;
  #ifdef DEBUG
      std::cout << "exp: " << exp << std::endl;
  #endif
    }
  #ifdef DEBUG
    std::cout << "n: " << n << "; m: " << m << std::endl;
  #endif
    return n > m + exp ? n : m + exp;
  }

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
