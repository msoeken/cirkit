#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE aigmeta

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/graph/one_bit_color_map.hpp>
#include <boost/graph/stoer_wagner_min_cut.hpp>
#include <boost/test/unit_test.hpp>

#include <classical/io/read_aigmeta.hpp>
#include <classical/utils/aig_to_graph.hpp>

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

  if ( exists( jsonname ) )
  {
    aigmeta meta;
    read_aigmeta( meta, jsonname.string() );

    std::cout << "Meta-data available:" << std::endl
              << meta << std::endl;
  }

  aiger * aig;
  aig = aiger_init();
  aiger_open_and_read_from_file( aig, filename.string().c_str() );

  aig_graph graph;
  aig_to_graph_settings settings;
  settings.dotname = "/tmp/test.dot";
  aig_to_graph( aig, graph, settings );

  BOOST_AUTO( parities, boost::make_one_bit_color_map( num_vertices( graph ), get( boost::vertex_index, graph ) ) );
  int w = boost::stoer_wagner_min_cut( graph, get( boost::edge_weight, graph ), boost::parity_map( parities ) );

  std::cout << "Set A:";
  for ( unsigned i = 0u; i < num_vertices( graph ); ++i )
  {
    if ( get( parities, i ) )
    {
      std::cout << " " << get( boost::vertex_name, graph )[i];
    }
  }
  std::cout << std::endl;

  std::cout << "Set B:";
  for ( unsigned i = 0u; i < num_vertices( graph ); ++i )
  {
    if ( !get( parities, i ) )
    {
      std::cout << " " << get( boost::vertex_name, graph )[i];
    }
  }
  std::cout << std::endl;

  std::cout << "Min-cut is " << w << std::endl;

  std::cout << "AIG #inputs:  " << aig->num_inputs << std::endl
            << "AIG #outputs: " << aig->num_outputs << std::endl;

  aiger_reset( aig );
}

// Local Variables:
// c-basic-offset: 2
// End:
