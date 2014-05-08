#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE esop_minimization

#include <boost/test/unit_test.hpp>

#include <classical/optimization/generate_exact_psdkro.hpp>

BOOST_AUTO_TEST_CASE(simple)
{
  using namespace revkit;

  generate_exact_psdkro_settings settings;
  settings.verbose = true;
  generate_exact_psdkro( "../test/example.pla", settings );
}

// Local Variables:
// c-basic-offset: 2
// End:
