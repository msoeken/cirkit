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
// End:
