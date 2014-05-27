#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE aigmeta

#include <boost/format.hpp>
#include <boost/test/unit_test.hpp>

#include <classical/io/read_aigmeta.hpp>

extern "C" {
#include <aiger.h>
}

BOOST_AUTO_TEST_CASE(simple)
{
  using boost::unit_test::framework::master_test_suite;
  using namespace revkit;

  aigmeta meta;
  read_aigmeta( meta, boost::str( boost::format( "%s.json" ) % master_test_suite().argv[1] ) );

  aiger * aig;
  aig = aiger_init();
  aiger_open_and_read_from_file( aig, boost::str( boost::format( "%s.aig" ) % master_test_suite().argv[1] ).c_str() );
  aiger_reset( aig );

  std::cout << meta << std::endl;

  std::cout << "AIG #inputs:  " << aig->num_inputs << std::endl
            << "AIG #outputs: " << aig->num_outputs << std::endl;
}

// Local Variables:
// c-basic-offset: 2
// End:
