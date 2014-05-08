#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE truth_table

#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp>

#include <core/truth_table.hpp>

BOOST_AUTO_TEST_CASE(simple)
{
  using boost::test_tools::output_test_stream;

  using namespace revkit;

  binary_truth_table spec;

  spec.add_entry( number_to_truth_table_cube( 0u, 2u ), number_to_truth_table_cube( 0u, 2u ) );
  spec.add_entry( number_to_truth_table_cube( 1u, 2u ), number_to_truth_table_cube( 1u, 2u ) );
  spec.add_entry( number_to_truth_table_cube( 2u, 2u ), number_to_truth_table_cube( 3u, 2u ) );
  spec.add_entry( number_to_truth_table_cube( 3u, 2u ), number_to_truth_table_cube( 2u, 2u ) );

  output_test_stream output;
  output << spec;

  BOOST_CHECK( !output.is_empty( false ) );
  BOOST_CHECK( output.is_equal( "00 00\n01 01\n10 11\n11 10\n" ) );
}
