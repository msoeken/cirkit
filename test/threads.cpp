#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE threads

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(simple)
{
  std::thread t1([]() {
      std::this_thread::sleep_for( std::chrono::seconds( 5u ) );
      std::cout << "Exit." << std::endl;
      exit( EXIT_FAILURE );
    });

  for ( unsigned i = 0u; i < (1u << 27u); ++i )
  {
    std::cout << "Loop " << i << std::endl;
  }

  t1.detach();
}

// Local Variables:
// c-basic-offset: 2
// End:
