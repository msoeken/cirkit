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
// End:
