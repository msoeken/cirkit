#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE aigmeta

#include <list>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/optional.hpp>
#include <boost/test/unit_test.hpp>

#include <classical/io/read_aigmeta.hpp>
#include <classical/utils/aiger_to_aig.hpp>
#include <classical/utils/find_mincut.hpp>

extern "C" {
#include <aiger.h>
}

BOOST_AUTO_TEST_CASE(simple)
{
  using boost::unit_test::framework::master_test_suite;
  using namespace revkit;

  assert( master_test_suite().argc == 2u );
  boost::filesystem::path filename( master_test_suite().argv[1] );
  boost::filesystem::path jsonname( boost::str( boost::format( "%s/%s.json" ) % filename.parent_path().string() % filename.stem().string() ) );

  boost::optional<aigmeta> meta;
  if ( exists( jsonname ) )
  {
    meta = aigmeta();
    read_aigmeta( *meta, jsonname.string() );

    std::cout << "Meta-data available:" << std::endl
              << *meta << std::endl;
  }

  aiger * aig;
  aig = aiger_init();
  aiger_open_and_read_from_file( aig, filename.string().c_str() );

  aig_graph graph;
  aiger_to_aig_settings settings;
  settings.dotname = "/tmp/test.dot";
  aiger_to_aig( aig, graph, settings );

  std::list<aig_function> cut;
  find_mincut( graph, cut );
  std::cout << "Found cut of size: " << cut.size() << std::endl;

  std::cout << "AIG #inputs:  " << aig->num_inputs << std::endl
            << "AIG #outputs: " << aig->num_outputs << std::endl;

  aiger_reset( aig );
}

// Local Variables:
// c-basic-offset: 2
// End:
