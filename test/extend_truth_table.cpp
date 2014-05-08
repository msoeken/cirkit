#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE extend_truth_table

#include <fstream>

#include <boost/test/unit_test.hpp>

#include <core/truth_table.hpp>
#include <core/functions/approximate_additional_lines.hpp>
#include <core/functions/extend_pla.hpp>
#include <core/io/read_pla.hpp>
#include <core/io/read_pla_to_bdd.hpp>
#include <core/io/write_pla.hpp>

BOOST_AUTO_TEST_CASE(simple)
{
  using boost::unit_test::framework::master_test_suite;
  using namespace revkit;

  if ( master_test_suite().argc != 2u ) return;

  binary_truth_table pla, extended;
  read_pla_settings settings;
  settings.extend = false;
  read_pla( pla, master_test_suite().argv[1], settings );
  extend_pla( pla, extended );

  write_pla( extended, "/tmp/test.pla" );
  unsigned lines = approximate_additional_lines( "/tmp/test.pla" );

  std::filebuf fb;
  fb.open( "/tmp/lines_extend_truth_table", std::ios::out );

  std::ostream os( &fb );
  os << lines << std::endl;
  fb.close();
}
