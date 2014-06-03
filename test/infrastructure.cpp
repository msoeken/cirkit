#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE infrastructure

#include <boost/test/unit_test.hpp>

#include <core/utils/benchmark_table.hpp>

BOOST_AUTO_TEST_CASE(simple)
{
  benchmark_table<std::string, unsigned, double> table( { "Name", "Gates", "Time" } );

  table.add( std::string( "hwb4" ),  10u, 4.50 );
  table.add( std::string( "sqrt2" ), 18u, 9.20 );
  table.add( std::string( "d2" ),     2u, 1.01 );

  table.print();
}

// Local Variables:
// c-basic-offset: 2
// End:
