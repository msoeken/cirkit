#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE esop_minimization

#include <boost/test/unit_test.hpp>

#include <classical/optimization/esop_minimization.hpp>

BOOST_AUTO_TEST_CASE(simple)
{
  using boost::unit_test::framework::master_test_suite;
  using namespace revkit;

  properties::ptr settings( new properties() );
  settings->set( "verbose", false );
  settings->set( "runs", 1u );
  settings->set( "verify", true );

  properties::ptr statistics( new properties() );

  if ( master_test_suite().argc == 2u )
  {
    esop_minimization( master_test_suite().argv[1], settings, statistics );
  }
  else
  {
    esop_minimization( "../test/example.pla", settings, statistics );
  }

  std::cout << "Number of cubes:    " << statistics->get<unsigned>( "cube_count" ) << std::endl
            << "Number of literals: " << statistics->get<unsigned>( "literal_count" ) << std::endl
            << "Run-time:           " << statistics->get<double>( "runtime" ) << std::endl;
}

// Local Variables:

// c-basic-offset: 2
// End:
