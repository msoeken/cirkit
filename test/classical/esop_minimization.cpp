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
