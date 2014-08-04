/* RevKit (www.revkit.org)
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
#define BOOST_TEST_MODULE threads

#include <iostream>
#include <memory>
#include <thread>

#include <boost/test/unit_test.hpp>

#include <core/utils/timeout.hpp>

using namespace cirkit;

BOOST_AUTO_TEST_CASE(simple)
{
  std::thread t1( []() { timeout_after( 5u ); } );

  for ( unsigned i = 0u; i < (1u << 27u); ++i )
  {
    std::cout << "Loop " << i << std::endl;
  }

  t1.detach();
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
