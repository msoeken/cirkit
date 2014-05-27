#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE aigmeta

#include <boost/test/unit_test.hpp>

#include <classical/io/read_aigmeta.hpp>

BOOST_AUTO_TEST_CASE(simple)
{
  using boost::unit_test::framework::master_test_suite;
  using namespace revkit;

  aigmeta meta;
  read_aigmeta( meta, master_test_suite().argv[1] );

  std::cout << meta << std::endl;
}

// Local Variables:
// c-basic-offset: 2
// End:
