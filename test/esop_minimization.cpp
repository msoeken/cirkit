#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE esop_minimization

#include <boost/test/unit_test.hpp>

#include <classical/optimization/generate_exact_psdkro.hpp>

BOOST_AUTO_TEST_CASE(simple)
{
  using boost::unit_test::framework::master_test_suite;
  using namespace revkit;

  generate_exact_psdkro_settings settings;
  settings.verbose = true;

  if ( master_test_suite().argc == 2u )
  {
    generate_exact_psdkro( master_test_suite().argv[1], settings );
  }
  else
  {
    generate_exact_psdkro( "../test/example.pla", settings );
  }
}

// Local Variables:
// c-basic-offset: 2
// End:
