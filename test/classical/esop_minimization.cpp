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
#define BOOST_TEST_MODULE esop_minimization

#include <boost/format.hpp>
#include <boost/test/unit_test.hpp>

#include <classical/optimization/esop_minimization.hpp>

#define COMPARE_WITH_EXORCISM 0

void on_cube( const cirkit::cube_t& cube )
{
  /* do nothing */
}

BOOST_AUTO_TEST_CASE(simple)
{
  using boost::unit_test::framework::master_test_suite;
  using namespace cirkit;

  properties::ptr settings( new properties() );
  settings->set( "verbose", true );
  settings->set( "runs", 1u );
  settings->set( "verify", true );
  settings->set( "on_cube", cube_function_t( on_cube ) );

  properties::ptr statistics( new properties() );

  std::string filename = ( master_test_suite().argc == 2u ) ? master_test_suite().argv[1] : "../test/example.pla";

  esop_minimization( filename, settings, statistics );

#if COMPARE_WITH_EXORCISM
  auto sresult = system( boost::str( boost::format( "(exorcism %s; echo) > /dev/null" ) % filename ).c_str() );

  std::cout << "EXORCISM cubes:     "; std::cout.flush();
  auto sresult = system( boost::str( boost::format( "cat %sesop | grep \"Final\" | awk '{print $6}' ") % filename.substr( 0u, filename.size() - 3u ) ).c_str() );
  std::cout << "EXORCISM literals:  "; std::cout.flush();
  auto sresult = system( boost::str( boost::format( "cat %sesop | grep \"Final\" | awk '{print $9}' ") % filename.substr( 0u, filename.size() - 3u ) ).c_str() );
#endif

  std::cout << "Number of cubes:    " << statistics->get<unsigned>( "cube_count" ) << std::endl
            << "Number of literals: " << statistics->get<unsigned>( "literal_count" ) << std::endl
            << "Run-time:           " << statistics->get<double>( "runtime" ) << std::endl;
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
