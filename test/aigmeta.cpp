#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE aigmeta

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/graph/boykov_kolmogorov_max_flow.hpp>
#include <boost/range/iterator_range.hpp>
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

  double flow = boykov_kolmogorov_max_flow( graph, 0, 1 );
  std::cout << "Total flow: " << flow << std::endl;

  auto capacity = boost::get( boost::edge_capacity, graph );
  auto name     = boost::get( boost::vertex_name,   graph );
  auto color    = boost::get( boost::vertex_color,  graph );

  for ( const auto& e : boost::make_iterator_range( edges( graph ) ) )
  {
    if ( capacity[e] > 0 )
    {
      if ( color[source(e, graph)] != color[target(e, graph)] )
      {
        std::cout << "Cut between " << name[source(e, graph)] << " and " << name[target(e, graph)] << std::endl;
      }
    }
  }

  std::cout << "AIG #inputs:  " << aig->num_inputs << std::endl
            << "AIG #outputs: " << aig->num_outputs << std::endl;

  aiger_reset( aig );
}

// Local Variables:
// c-basic-offset: 2
// End:
