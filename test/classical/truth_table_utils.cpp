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

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE truth_table_utils

#include <boost/test/unit_test.hpp>

#if ADDON_EXPERIMENTAL

#include <iostream>
#include <classical/utils/truth_table_utils.hpp>

BOOST_AUTO_TEST_CASE(simple)
{
  using namespace cirkit;

  tt a = tt_nth_var( 0u );
  tt b = tt_nth_var( 1u );
  tt c = tt_nth_var( 2u );

  std::cout << "a: " << a << std::endl;
  std::cout << "b: " << b << std::endl;
  std::cout << "c: " << c << std::endl;

  tt_align( a, c );
  tt_align( b, c );

  std::cout << "a: " << a << std::endl;
  std::cout << "b: " << b << std::endl;
  std::cout << "c: " << c << std::endl;

  std::cout << "a & c: " << (a & c) << std::endl;
  std::cout << "a & b: " << (a & b) << std::endl;
  std::cout << "a | b | c: " << (a | b | c) << std::endl;
  std::cout << "!(a & b): " << ~(a & b) << std::endl;
}

#else

BOOST_AUTO_TEST_CASE(simple) {}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
