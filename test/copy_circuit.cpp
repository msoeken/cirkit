#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE truth_table

#include <boost/assign/std/vector.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp>

#include <reversible/circuit.hpp>
#include <reversible/functions/add_gates.hpp>
#include <reversible/functions/copy_circuit.hpp>
#include <reversible/io/print_circuit.hpp>

BOOST_AUTO_TEST_CASE(simple)
{
  using boost::test_tools::output_test_stream;

  using namespace boost::assign;
  using namespace revkit;

  circuit circ( 6u ), copy, smaller_copy;

  append_toffoli( circ )( 0u, 2u )( 5u );
  append_cnot( circ, 0u, 2u );
  append_not( circ, 0u );

  output_test_stream output;
  output << circ;

  BOOST_CHECK( !output.is_empty( false ) );
  BOOST_CHECK( output.is_equal( "**O\n---\n*O-\n---\n---\nO--\n" ) );

  copy_circuit( circ, copy );

  output << copy;

  BOOST_CHECK( !output.is_empty( false ) );
  BOOST_CHECK( output.is_equal( "**O\n---\n*O-\n---\n---\nO--\n" ) );

  std::vector<unsigned> filter;
  filter += 0u,2u,5u;

  copy_circuit( circ, smaller_copy, filter );

  output << smaller_copy;

  BOOST_CHECK( !output.is_empty( false ) );
  BOOST_CHECK( output.is_equal( "**O\n*O-\nO--\n" ) );
}
