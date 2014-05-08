#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE circuit_io

#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp>

#include <core/circuit.hpp>
#include <core/functions/add_gates.hpp>
#include <core/io/print_circuit.hpp>
#include <core/io/read_realization.hpp>
#include <core/io/write_realization.hpp>
#include <core/io/write_verilog.hpp>

BOOST_AUTO_TEST_CASE(simple)
{
  using boost::test_tools::output_test_stream;

  using namespace revkit;

  circuit circ( 3u ), circ2;
  std::string circ_str = "―⊕―○―――●―\n―――⊕―●―⨯―\n―○―●―⊕―⨯―\n";
  output_test_stream output, output2;

  /* create circuit */
  append_cnot( circ, make_var( 2u, false ), 0u );
  append_toffoli( circ )( make_var( 0u, false ), make_var( 2u ) )( 1u );
  append_cnot( circ, 1u, 2u );
  append_fredkin( circ )( make_var( 0u ) )( 1u, 2u );

  /* print circuit */
  output << circ;
  BOOST_CHECK( output.is_equal( circ_str ) );

  /* write and read circuit */
  write_realization( circ, "/tmp/test.real" );
  read_realization( circ2, "/tmp/test.real" );

  /* print read circuit */
  output2 << circ;
  BOOST_CHECK( output2.is_equal( circ_str ) );

  /* write verilog */
  write_verilog( circ, "/tmp/test.v" );
}

// Local Variables:
// c-basic-offset: 2
// End:
