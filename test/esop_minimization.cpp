#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE esop_minimization

#include <boost/format.hpp>
#include <boost/test/unit_test.hpp>

#include <classical/optimization/esop_minimization.hpp>

#define COMPARE_WITH_EXORCISM 1

BOOST_AUTO_TEST_CASE(simple)
{
  using boost::unit_test::framework::master_test_suite;
  using namespace revkit;

  properties::ptr settings( new properties() );
  settings->set( "verbose", false );
  settings->set( "runs", 1u );
  settings->set( "verify", true );

  properties::ptr statistics( new properties() );

  std::string filename = ( master_test_suite().argc == 2u ) ? master_test_suite().argv[1] : "../test/example.pla";

  esop_minimization( filename, settings, statistics );

#if COMPARE_WITH_EXORCISM
  system( boost::str( boost::format( "(exorcism %s; echo) > /dev/null" ) % filename ).c_str() );

  std::cout << "EXORCISM cubes:     "; std::cout.flush();
  system( boost::str( boost::format( "cat %sesop | grep \"Final\" | awk '{print $6}' ") % filename.substr( 0u, filename.size() - 3u ) ).c_str() );
  std::cout << "EXORCISM literals:  "; std::cout.flush();
  system( boost::str( boost::format( "cat %sesop | grep \"Final\" | awk '{print $9}' ") % filename.substr( 0u, filename.size() - 3u ) ).c_str() );
#endif

  std::cout << "Number of cubes:    " << statistics->get<unsigned>( "cube_count" ) << std::endl
            << "Number of literals: " << statistics->get<unsigned>( "literal_count" ) << std::endl
            << "Run-time:           " << statistics->get<double>( "runtime" ) << std::endl;
}

// Local Variables:

// c-basic-offset: 2
// End:
