#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE truth_table_utils

#include <boost/test/unit_test.hpp>

#if ADDON_EXPERIMENTAL

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
// End:
