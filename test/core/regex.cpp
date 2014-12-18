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
#define BOOST_TEST_MODULE regex

#include <regex>

#include <boost/config.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(simple)
{

#if defined(BOOST_CLANG) || ( defined(BOOST_GCC) && BOOST_GCC >= 40900 )
  std::regex r( "\\s+" );
  BOOST_CHECK( std::regex_replace( "Test String", r, " " )               == "Test String" );
  BOOST_CHECK( std::regex_replace( "Test  String", r, " " )              == "Test String" );
  BOOST_CHECK( std::regex_replace( "Another   Test     String", r, " " ) == "Another Test String" );
  BOOST_CHECK( std::regex_replace( "Test    String  ", r, " " )          == "Test String " );
#endif

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
